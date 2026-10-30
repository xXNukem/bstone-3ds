#ifndef BSTONE_3DS_HUD_INCLUDED
#define BSTONE_3DS_HUD_INCLUDED

#if defined(__3DS__) || defined(_3DS)

#include "3d_def.h"
#include "bstone_3ds_automap.h"
#include "bstone_3ds_messages.h"
#include "bstone_3ds_intro.h"
#include <citro2d.h>
#include <cstdio>
#include <string>
#include <vector>

namespace bstone {
namespace n3ds_hud {

void draw_frame_screw(int x, int y);

void draw_watch_frame();

void draw_screen_background();

u32 crt_background();

u32 crt_panel();

u32 crt_border();

u32 crt_dim();

u32 crt_green();

u32 crt_bright();

const std::vector<std::uint8_t>& hud_font();

int text_width(const char* value);

void draw_text(
    int x,
    int y,
    const char* value,
    u32 color,
    int scale = 1);

void draw_clipped_text(
    int x,
    int y,
    const char* value,
    u32 color,
    int top,
    int bottom,
    int right);

void draw_monitor_outline(
    int x, int y, int width, int height);

void draw_ecg();

void draw_info();

void draw_status();

void draw_interference();

void draw();

} // namespace n3ds_hud
} // namespace bstone

#endif // 3DS
#endif // BSTONE_3DS_HUD_INCLUDED
