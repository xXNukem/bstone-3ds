// SPDX-License-Identifier: GPL-2.0-or-later
#include "bstone_3ds_menu.h"
#include "3d_menu.h"
#include "bstone_3ds_intro.h"
#include "id_vl.h"
#include <3ds.h>
#include <algorithm>
#include <array>
#include <cmath>

namespace bstone { namespace n3ds_menu {
namespace {
const CP_iteminfo* menu = nullptr;
const CP_itemtype* entries = nullptr;
int selected = 0;
bool panel_active = false;
int loading_depth = 0;
bool loading_owned_panel = false;
bool loading_suspended = false;
bool leaving = false;
std::string header;
std::string next_header;
bool header_pending = false;
std::uint64_t frame_ms = 0;
std::uint64_t last_present_ms = 0;
bool paused = false;
bool planet_strike = false;
std::string footer = "A SELECT   B BACK   D-PAD NAVIGATE";
bool document_mode = false;
bool painting_text = false;
bool meters = false;
bool content_pending = false;
int document_scroll = 0;
int document_index = -1;
int document_count = 1;
std::uint64_t scroll_time = 0;
std::uint32_t canvas_revision = 0;
std::array<std::uint8_t, 320 * 200> ink{};
std::array<int, 2> levels{};
int meter_selected = 0;
int meter_max = 1;
float layer = 0.7f;
float eye_strength = 0.0f;
float view_translation = 0.0f;

void plane(float depth)
{
    const float shift = eye_strength * depth;
    if (shift != view_translation) { C2D_ViewTranslate(shift - view_translation, 0.0f); }
    view_translation = shift;
}

// The indexed content is still 320 x 200, scaled by the existing presenter
// to 400 x 240. Keep its viewport separate from native header/footer baselines.
constexpr int content_top = 28; // Native y ~= 34, below the compact header.
constexpr int content_bottom = 179; // Native y < 216, above the help baseline.
constexpr int document_source_top = 8;
constexpr int document_source_bottom = 175;
constexpr int document_view_height = content_bottom - content_top + 1;
constexpr int document_max_scroll =
    (document_source_bottom - document_source_top + 1) - document_view_height;

// Native 400 x 240 coordinates, shared phosphor palette with the 3DS intro.
const u32 track = C2D_Color32(25, 67, 36, 255);
const u32 bright = C2D_Color32(168, 255, 179, 255);
const u32 green = C2D_Color32(88, 235, 121, 255);
const u32 dim = C2D_Color32(76, 153, 93, 255);

void bar(float x, float y, float w, float h, u32 color)
{
    C2D_DrawRectSolid(x, y, layer, w, h, color);
}

void text(int x, int y, const char* value, u32 color)
{
    n3ds_intro::draw_font(x, y, value, n3ds_intro::fonts().menu, 1, color);
}

int text_width(const char* value)
{
    return n3ds_intro::font_text_width(n3ds_intro::fonts().menu, value);
}

// These are native presentation coordinates, NOT the engine's 320 x 200
// indexed UI coordinates. Measure with exactly the font and scale used by text.
int centered_x(const char* value)
{
    return (400 - text_width(value)) / 2;
}

void centered(int y, const char* value, u32 color)
{
    text(centered_x(value), y, value, color);
}

void row(int y, const CP_itemtype& entry, const std::string& label_text, bool highlight)
{
    if (label_text.empty()) { return; }
    const auto* label = label_text.c_str();
    const int x = centered_x(label);
    const int width = text_width(label);
    const int marker_width = text_width(">");
    constexpr int gap = 6;
    constexpr int padding = 5;
    const int left = x - gap - marker_width - padding;
    const int right = x + width + gap + marker_width + padding;
    plane(highlight ? 0.65f : 0.0f);
    if (highlight)
    {
        // Symmetric padding leaves the label itself centered at x=200.
        bar(left, y - 2, right - left, 13, C2D_Color32(16, 57, 28, 255));
        bar(left, y - 2, 2, 13, green);
        text(x - gap - marker_width, y, ">", bright);
    }
    centered(y, label, entry.active == AT_DISABLED ? dim :
        (highlight ? bright : green));
}

void crt_background(int x, int y, int width, int height)
{
    // Time-based, bounded work: no textures, per-frame allocations or RNG.
    // Only the dark background varies; labels retain stable phosphor colors.
    if (!vid_cfg_menu_crt())
    {
        bar(x, y, width, height, C2D_Color32(2, 9, 5, 255));
        return;
    }
    const double seconds = frame_ms * 0.001;
    // Smooth deterministic noise shared by both eyes; no event-driven phase.
    const std::uint32_t bucket = static_cast<std::uint32_t>(frame_ms / 240);
    const auto noise = [](std::uint32_t value)
    {
        value = value * 1664525u + 1013904223u;
        return double((value >> 16) & 255u) / 255.0 - 0.5;
    };
    double blend = double(frame_ms % 240) / 240.0;
    blend = blend * blend * (3.0 - 2.0 * blend);
    const double flutter = noise(bucket) * (1.0 - blend) + noise(bucket + 1) * blend;
    const double drift = 1.2 * std::sin(seconds * 0.8) + 0.5 * std::sin(seconds * 2.1);
    const int glow = static_cast<int>(std::lround(11.0 + drift + flutter * 0.8));
    bar(x, y, width, height, C2D_Color32(2, glow, 5, 255));
    const float scan_phase = float(std::sin(seconds * 0.55) * 0.35);
    for (int line = y; line < y + height; line += 4)
    {
        bar(x, line + scan_phase, width, 1, C2D_Color32(12, 29, 17, 105));
    }
    const float sweep = float(std::fmod(seconds * 24.0, 288.0) - 24.0);
    for (int band = -12; band <= 12; ++band)
    {
        const float row = sweep + band * 2.0f;
        if (row >= y && row < y + height)
        {
            const float radius = band / 12.0f;
            const float intensity = (1.0f - radius * radius) * (1.0f - radius * radius);
            bar(x, row, width, std::min(2.0f, float(y + height) - row),
                C2D_Color32(12, 50, 23, int(65.0f * intensity)));
        }
    }
    // Occasional sparse grain, not a continually flashing noise field.
    if ((frame_ms / 100) % 11 == 0)
    {
        for (unsigned i = 0; i < 4; ++i)
        {
            const unsigned seed = bucket * 1664525u + i * 1013904223u;
            bar(x + int(seed % unsigned(width)), y + int((seed >> 12) % unsigned(height)),
                1, 1, C2D_Color32(30, 65, 37, 45));
        }
    }
}

void draw_header(const char* title)
{
    // No opaque title band, border, corner marks or divider: the whole display
    // is the terminal. The CRT background remains continuous behind the header.
    centered(12, title, bright);
    const char* status = paused ? "PAUSED" : "ONLINE";
    text(400 - 12 - text_width(status), 12, status, dim);
}
}

void begin(bool is_ps, bool is_paused)
{
    end();
    n3ds_intro::cache_fonts();
    panel_active = true;
    planet_strike = is_ps;
    paused = is_paused;
    last_present_ms = osGetTime();
    clear_content();
}

void end()
{
    hide();
    panel_active = false;
    header.clear();
    next_header.clear();
    header_pending = false;
    document_mode = false;
    meters = false;
    ++canvas_revision;
}

bool active() { return panel_active; }

void set_header(const char* title)
{
    if (!panel_active) { return; }
    next_header = title;
    for (auto& ch : next_header)
    {
        if (ch >= 'a' && ch <= 'z') { ch -= 'a' - 'A'; }
    }
    if (next_header == "MAIN OPTIONS") { next_header = "MAIN TERMINAL"; }
    else if (next_header == "LOAD MISSION") { next_header = "LOAD GAME"; }
    else if (next_header == "SAVE MISSION") { next_header = "SAVE GAME"; }
    else if (next_header == "GAME OPTIONS") { next_header = "OPTIONS"; }
    else if (next_header == "CONTROL") { next_header = "CONTROL SETTINGS"; }
    else if (next_header == "CHOOSE A MISSION") { next_header = "NEW GAME"; }
    else if (next_header == "DIFFICULTY LEVEL") { next_header = "DIFFICULTY"; }
    header_pending = true;
}

void begin_eye(int eye, float slider)
{
    plane(0.0f);
    const float strength = panel_active && slider > 0.01f ?
        std::max(0.0f, std::min(1.0f, slider)) : 0.0f;
    eye_strength = (eye == 0 ? 1.0f : -1.0f) * strength;
}

float content_shift()
{
    if (!panel_active) { return 0.0f; }
    // Long documents share one conservative front reading plane. Interactive
    // menus and the loading display retain their approved depth unchanged.
    return eye_strength * (document_mode && !loading() ? 0.25f : -0.35f);
}

void begin_frame()
{
    if (!panel_active) { return; }
    frame_ms = osGetTime();
    last_present_ms = frame_ms;
}

bool needs_refresh()
{
    return panel_active && (vid_cfg_menu_crt() || loading()) && osGetTime() - last_present_ms >= 16;
}

void idle()
{
    if (!panel_active) { return; }
    if (needs_refresh()) { VL_RefreshScreen(); return; }
    // Sleep toward the next visual deadline, with short slices for input.
    // This also bridges the engine's 70-Hz tick to the 16-ms UI frame interval.
    const auto elapsed = osGetTime() - last_present_ms;
    const auto remaining = vid_cfg_menu_crt() || loading() ? (elapsed < 16 ? 16 - elapsed : 1) : 4;
    const auto wait_ms = std::min<std::uint64_t>(4, remaining);
    svcSleepThread(static_cast<std::int64_t>(wait_ms) * 1000000LL);
    if (needs_refresh()) { VL_RefreshScreen(); }
}

bool loading() { return loading_depth > 0 && !loading_suspended; }

void begin_loading()
{
    if (loading_depth > 0) { ++loading_depth; return; }
    loading_owned_panel = !panel_active;
    if (loading_owned_panel) { begin(get_assets_info().is_ps(), false); }
    loading_suspended = false;
    loading_depth = 1;
    ++canvas_revision;
    // Submit both screens before any blocking work. Do not publish or erase
    // the caller's menu descriptor simply to show the loading overlay.
    VL_RefreshScreen();
}

void end_loading()
{
    if (loading_depth <= 0 || --loading_depth > 0) { return; }
    ++canvas_revision;
    if (loading_owned_panel) { end(); }
    loading_owned_panel = false;
    // No present here: keep the last loading frame until the destination is
    // complete and publishes through its existing update path.
}

void loading_step()
{
    if (loading() && needs_refresh()) { VL_RefreshScreen(); }
}

void suspend_loading()
{
    // Error dialogs must become visible before their acknowledgement wait.
    if (loading_depth > 0) { loading_suspended = true; ++canvas_revision; }
}

void draw_bottom()
{
    if (!panel_active) { return; }
    if (!loading()) { n3ds_intro::draw_menu_title(planet_strike); return; }
    bar(0, 0, 320, 240, C2D_Color32(2, 9, 5, 255));
    constexpr double pi = 3.141592653589793;
    const double phase = double(frame_ms % 1800) / 1800.0 * 24.0;
    for (int segment = 0; segment < 24; ++segment)
    {
        const double angle = (segment / 24.0) * 2.0 * pi - pi / 2.0;
        const double light = 0.5 + 0.5 * std::cos((segment - phase) * 2.0 * pi / 24.0);
        const double strength = light * light * light * light;
        const u32 color = C2D_Color32(24 + int(64 * strength), 48 + int(187 * strength),
            32 + int(89 * strength), 255);
        C2D_DrawLine(float(160 + std::cos(angle) * 51), float(114 + std::sin(angle) * 51), color,
            float(160 + std::cos(angle) * 72), float(114 + std::sin(angle) * 72), color, 7, 0.7f);
    }
    n3ds_intro::draw_centered("PLEASE WAIT", 209, false, 1, dim, false);
}

void show(const CP_iteminfo& info, const CP_itemtype* items, bool is_paused, bool is_ps)
{
    n3ds_intro::cache_fonts();
    leaving = false;
    header = "MAIN TERMINAL";
    next_header.clear();
    header_pending = false;
    show_list(info, items);
    paused = is_paused;
    planet_strike = is_ps;
}

void select(int index) { selected = index; }
void leave_on_next_update() { leaving = menu != nullptr; }

void hide() { menu = nullptr; entries = nullptr; leaving = false; }

void publish_destination()
{
    if (leaving || content_pending)
    {
        // Palette-only presentations keep the old native screen. Hand off only
        // with finished destination content, never the fade's final flat color.
        if (screenfaded)
        {
            VL_SetPalette(0, 256, vgapal);
            screenfaded = false;
        }
        if (leaving) { hide(); }
        content_pending = false;
    }
    if (header_pending)
    {
        header = next_header;
        header_pending = false;
    }
}

void clear_content()
{
    if (!panel_active) { return; }
    hide();
    document_mode = false;
    meters = false;
    document_scroll = 0;
    document_index = -1;
    content_pending = true;
    painting_text = false;
    ink.fill(0);
    std::fill(vid_ui_buffer_.begin(), vid_ui_buffer_.end(), 0);
    vid_mask_buffer_.fill(false);
    footer = "A SELECT   B BACK   D-PAD NAVIGATE";
    ++canvas_revision;
}

void set_footer(const char* value) { footer = value; }

void show_list(const CP_iteminfo& info, const CP_itemtype* items)
{
    menu = &info;
    entries = items;
    selected = info.curpos;
    leaving = false;
    document_mode = false;
    meters = false;
    ++canvas_revision;
}

void show_document(const char* title, bool can_continue)
{
    clear_content();
    document_mode = true;
    set_header(title);
    set_footer(can_continue ? "D-PAD SCROLL/PAGE   A CONTINUE   B BACK" :
        "UP/DN SCROLL   LEFT/RIGHT PAGE   B BACK");
}

bool document() { return panel_active && document_mode; }
bool accepts_text() { return !menu && !meters; }
void text_paint(bool enabled) { painting_text = enabled; }

void pixel(int x, int y)
{
    if (x < 0 || x >= 320 || y < 0 || y >= 200) { return; }
    ink[y * 320 + x] = painting_text ? 1 : 0;
    ++canvas_revision;
}

void clear_ink(int x, int y, int width, int height)
{
    for (int row = std::max(y, 0); row < std::min(y + height, 200); ++row)
    {
        for (int col = std::max(x, 0); col < std::min(x + width, 320); ++col)
        {
            ink[row * 320 + col] = 0;
        }
    }
    ++canvas_revision;
}

std::uint32_t revision() { return canvas_revision; }

std::uint32_t sample_ui(int x, int y, const std::uint32_t* palette)
{
    if (loading() || y < content_top || y > content_bottom) { return 0; }
    if (document_mode)
    {
        if (x < 8 || x > 311) { return 0; }
        y = document_source_top + (y - content_top) + document_scroll;
    }
    if (x < 0 || x >= 320 || y < 0 || y >= 200) { return 0; }
    const int i = y * 320 + x;
    if (!vid_mask_buffer_[i]) { return 0; }
    const std::uint32_t color = ink[i] ? 0x58EB79 : palette[vid_ui_buffer_[i]];
    return (color << 8) | 255;
}

void document_page(int index, int count)
{
    if (!document_mode) { return; }
    if (document_index != index)
    {
        document_index = index;
        document_scroll = 0;
        ++canvas_revision;
    }
    document_count = count;
}

bool scroll_document(int direction)
{
    if (!document_mode) { return false; }
    const int next = std::max(0, std::min(document_max_scroll, document_scroll + direction * 8));
    if (next == document_scroll) { return false; }
    const auto now = osGetTime();
    if (now >= scroll_time)
    {
        document_scroll = next;
        scroll_time = now + 140;
        ++canvas_revision;
    }
    return true;
}

void show_meters(int which, int sfx, int music, int maximum)
{
    menu = nullptr;
    meters = true;
    meter_selected = which;
    levels = {{sfx, music}};
    meter_max = std::max(maximum, 1);
    ++canvas_revision;
    set_footer("UP/DN SELECT   LEFT/RIGHT VOLUME   B BACK");
}

void draw_background()
{
    if (!panel_active) { return; }
    layer = 0.55f; // Behind the existing UI texture (0.6), without a new renderer.
    // Overscan the background only when shifted; no exposed edge or border.
    if (eye_strength != 0.0f)
    {
        bar(0, 0, 400, 240, C2D_Color32(2, 9, 5, 255));
        plane(-1.25f);
        crt_background(-3, 0, 406, 240);
    }
    else { crt_background(0, 0, 400, 240); }
    plane(0.0f);
    layer = 0.7f;
}

void draw_ambient_background(int width, int height, std::uint64_t milliseconds)
{
    const auto saved_layer = layer;
    const auto saved_time = frame_ms;
    layer = 0.55f; frame_ms = milliseconds;
    crt_background(0, 0, width, height);
    layer = saved_layer; frame_ms = saved_time;
}

void draw()
{
    if (!panel_active) { return; }
    plane(document_mode && !loading() ? 0.0f : -0.35f);
    if (loading())
    {
        const auto& face = n3ds_intro::fonts().menu;
        const int width = n3ds_intro::font_text_width(face, "LOADING...");
        const auto* font = reinterpret_cast<const fontstruct*>(face.data());
        n3ds_intro::draw_font((400 - width * 2) / 2, (240 - font->height * 2) / 2,
            "LOADING...", face, 2, bright);
        plane(0.0f);
        return;
    }
    draw_header(header.empty() ? "INFORMATION" : header.c_str());
    plane(0.0f);
    int help_y = 225;
    if (menu)
    {
        int count = 0;
        for (int i = 0; i < menu->amount; ++i)
        {
            if (!cp_get_terminal_label(*menu, entries, i).empty()) { ++count; }
        }
        const int spacing = count > 8 ? 16 : 20;
        const int first_y = 40 + (172 - count * spacing) / 2;
        int visible = 0;
        for (int i = 0; i < menu->amount; ++i)
        {
            const auto label = cp_get_terminal_label(*menu, entries, i);
            if (label.empty()) { continue; }
            row(first_y + visible * spacing, entries[i], label, selected == i);
            ++visible;
        }
        help_y = first_y + std::max(visible - 1, 0) * spacing + 20;
    }
    if (meters)
    {
        for (int i = 0; i < 2; ++i)
        {
            plane(i == meter_selected ? 0.65f : 0.0f);
            const int y = 82 + i * 60;
            centered(y - 16, i == 0 ? "SOUND EFFECTS" : "MUSIC", i == meter_selected ? bright : green);
            bar(84, y, 232, 8, track);
            bar(86, y + 2, 228 * levels[i] / meter_max, 4, i == meter_selected ? bright : green);
            const auto value = std::to_string(levels[i]) + " / " + std::to_string(meter_max);
            centered(y + 13, value.c_str(), dim);
        }
    }
    // Document header, page counter and help stay at the screen plane, behind
    // the slightly forward body. Menu chrome retains its existing plane.
    plane(document_mode ? 0.0f : -0.35f);
    if (document_mode)
    {
        const auto position = "PAGE " + std::to_string(document_index + 1) + " / " +
            std::to_string(document_count);
        text(12, 12, position.c_str(), dim);
    }
    centered(help_y, footer.c_str(), dim);
    plane(0.0f);
}
} }
