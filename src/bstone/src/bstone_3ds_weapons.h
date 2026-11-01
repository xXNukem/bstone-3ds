// Optional local AI artwork pack. No gameplay/animation state lives here.
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef BSTONE_3DS_WEAPONS_INCLUDED
#define BSTONE_3DS_WEAPONS_INCLUDED
#include <array>
#include <cstdint>
#include <string>
#include <vector>
namespace bstone { namespace n3ds_weapons {
enum class Game : std::uint16_t { aog = 1, planet_strike = 2 };
struct Frame
{
    int left{}, top{}, width{}, height{};
    int dimension{}, anchor_x{}, anchor_y{}, ref_height{}, offset_x{}, offset_y{};
    std::vector<std::int16_t> pixels; // trimmed, column major, -1 transparent
    const std::int16_t* get_column(int x) const { return pixels.data() + x * height; }
};
// Call after edition-specific original sprite constants and preferences exist.
void initialize(const std::string& data_directory, Game game, const std::array<int, 6>& bases);
void refresh(); // startup / entering Video Settings / explicit enable retry only
bool available() noexcept;
const char* status() noexcept;
void release() noexcept;
const Frame* get_frame(int original_sprite_id, bool enabled) noexcept;
} }
#endif
