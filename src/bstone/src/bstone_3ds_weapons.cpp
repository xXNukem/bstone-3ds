// Optional local AI artwork pack. Original VSWAPs are never written.
// SPDX-License-Identifier: GPL-2.0-or-later
#include "bstone_3ds_weapons.h"
#include "bstone_crc32.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <memory>
#include <utility>

namespace bstone { namespace n3ds_weapons {
namespace {
struct Weapon { std::array<Frame, 5> frames; };
std::string directory;
Game game{};
std::array<int, 6> sprite_bases{};
std::array<bool, 6> valid{};
std::unique_ptr<Weapon> cached;
int cached_slot = -1;
const char* notice = "NO ENHANCED PACK FOR THIS GAME";
constexpr const char* names[] = {"autocharge", "protector", "pulse", "ion", "grenade", "bfg"};
unsigned u16(const std::uint8_t* p) { return unsigned(p[0]) | (unsigned(p[1]) << 8); }
std::uint32_t u32(const std::uint8_t* p) { return u16(p) | (std::uint32_t(u16(p + 2)) << 16); }
int s16(const std::uint8_t* p) { const int n = u16(p); return n < 32768 ? n : n - 65536; }

std::unique_ptr<Weapon> load(int slot)
{
    const auto path = directory + "/" + names[slot] + ".bsw";
    std::unique_ptr<std::FILE, int(*)(std::FILE*)> file(std::fopen(path.c_str(), "rb"), std::fclose);
    if (!file || std::fseek(file.get(), 0, SEEK_END) != 0) { return {}; }
    const auto length = std::ftell(file.get());
    if (length < 72 || length > 32 + 5 * (8 + 128 * 128 * 2) ||
        std::fseek(file.get(), 0, SEEK_SET) != 0) { return {}; }
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(length));
    if (std::fread(bytes.data(), 1, bytes.size(), file.get()) != bytes.size()) { return {}; }
    const auto* p = bytes.data();
    if (std::memcmp(p, "BS3DAIW\0", 8) != 0 || u16(p + 8) != 1 ||
        u16(p + 10) != static_cast<unsigned>(game) || u16(p + 12) != unsigned(slot) ||
        u16(p + 14) != 5 || u16(p + 16) != 128) { return {}; }
    const int ax = u16(p + 18), ay = u16(p + 20), size = u16(p + 22);
    const int dx = s16(p + 24), dy = s16(p + 26);
    if (ax < 16 || ax > 112 || ay < 64 || ay > 127 || size < 32 || size > 180 ||
        std::abs(dx) > 32 || std::abs(dy) > 32) { return {}; }
    Crc32 crc;
    crc.update(p, 28);
    crc.update(p + 32, length - 32);
    if (crc.get_value() != u32(p + 28)) { return {}; }
    auto weapon = std::unique_ptr<Weapon>(new Weapon{});
    std::size_t at = 32;
    for (auto& frame : weapon->frames)
    {
        if (at + 8 > bytes.size()) { return {}; }
        frame.left = u16(p + at); frame.top = u16(p + at + 2);
        frame.width = u16(p + at + 4); frame.height = u16(p + at + 6); at += 8;
        if (frame.width < 1 || frame.height < 1 || frame.left + frame.width > 128 ||
            frame.top + frame.height > 128) { return {}; }
        const auto count = frame.width * frame.height;
        if (at + count * 2 > bytes.size()) { return {}; }
        frame.pixels.resize(count);
        bool opaque = false;
        for (auto& pixel : frame.pixels)
        {
            const auto value = u16(p + at); at += 2;
            if (value != 65535 && value > 255) { return {}; }
            pixel = value == 65535 ? -1 : static_cast<std::int16_t>(value);
            opaque |= pixel >= 0;
        }
        if (!opaque) { return {}; }
        frame.dimension = 128; frame.anchor_x = ax; frame.anchor_y = ay;
        frame.ref_height = size; frame.offset_x = dx; frame.offset_y = dy;
    }
    // A truncated/missing/extra phase invalidates the ENTIRE weapon.
    if (at != bytes.size()) { return {}; }
    return weapon;
}
}

void release() noexcept { cached.reset(); cached_slot = -1; }
bool available() noexcept { return std::any_of(valid.begin(), valid.end(), [](bool v) { return v; }); }
const char* status() noexcept { return notice; }
void refresh()
{
    release(); valid.fill(false);
    if (directory.empty()) { return; }
    for (int i = 0; i < 6; ++i)
    {
        if (sprite_bases[i] <= 0 || (game == Game::aog && i == 5)) { continue; }
        try { valid[i] = bool(load(i)); } catch (...) { valid[i] = false; }
    }
    notice = available() ? "ENHANCED PACK READY" : "PACK MISSING, INCOMPLETE OR INVALID";
}
void initialize(const std::string& data_directory, Game selected_game, const std::array<int, 6>& bases)
{
    game = selected_game; sprite_bases = bases;
    directory = data_directory;
    if (!directory.empty() && directory.back() != '/') { directory += '/'; }
    directory += "ai-weapons/";
    directory += game == Game::aog ? "aog" : "ps";
    refresh();
}
const Frame* get_frame(int sprite_id, bool enabled) noexcept
{
    if (!enabled) { return nullptr; }
    for (int slot = 0; slot < 6; ++slot)
    {
        const int phase = sprite_id - sprite_bases[slot];
        if (!valid[slot] || phase < 0 || phase >= 5) { continue; }
        if (cached_slot != slot)
        {
            // One weapon resident; load all five phases transactionally. No PNG
            // decoding, allocation or SD access in the steady-state frame loop.
            try
            {
                auto next = load(slot);
                if (!next)
                {
                    valid[slot] = false; release();
                    notice = "ENHANCED WEAPON UNAVAILABLE - ORIGINAL";
                    return nullptr;
                }
                cached = std::move(next); cached_slot = slot;
            }
            catch (...)
            {
                valid[slot] = false; release();
                notice = "ENHANCED WEAPON UNAVAILABLE - ORIGINAL";
                return nullptr;
            }
        }
        return &cached->frames[phase];
    }
    return nullptr;
}
} }
