/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BSTONE_ID_VL_INCLUDED
#define BSTONE_ID_VL_INCLUDED

#include <cstdint>
#include <memory>

#include <array>
#include <vector>

#include "bstone_extent_2d.h"
#include "bstone_ref_values.h"
#include "bstone_rgb_palette.h"
#include "bstone_cvar_mgr.h"
#include "bstone_string_view.h"
#include "bstone_fizzle_fx.h"

namespace bstone
{

class TextWriter;
class SpriteCache;

} // bstone

struct statobj_t;
struct objtype;

using VgaColor = std::array<std::uint8_t, 3>;
using VgaPalette = std::array<VgaColor, 256>;

using VgaBuffer = std::vector<std::uint8_t>;
using UiMaskBuffer = std::array<bool, vga_ref_width * vga_ref_height>;

constexpr int vid_native_width = 400;
constexpr int vid_native_height = 240;

extern std::uint8_t* vga_memory;

struct VidLayout
{
	int width;
	int height;

	int screen_width;
	int screen_height;

	int window_width;
	int window_height;

	int window_viewport_left_width;
	int window_viewport_right_width;
	int window_viewport_top_height;
	int window_viewport_bottom_height;

	int screen_left_filler_width;
	int screen_right_filler_width;
	int screen_top_filler_height;
	int screen_bottom_filler_height;
	int screen_width_4x3;

	int screen_viewport_left_width;
	int screen_viewport_right_width;
	int screen_viewport_width;

	int screen_viewport_top_height;
	int screen_viewport_bottom_height;
	int screen_viewport_height;
}; // VidLayout

extern VidLayout vid_layout_;

using ScreenshotBuffer = std::unique_ptr<std::uint8_t[]>;

using PackedPalette = std::array<std::uint32_t, bstone::RgbPalette::get_max_color_count()>;

extern VgaBuffer vid_ui_buffer_;
extern UiMaskBuffer vid_mask_buffer_;

extern bool vid_is_take_screenshot_scheduled;

struct CalculateScreenSizeInputParam
{
	bool is_widescreen;

	int width;
	int height;

	int window_width;
	int window_height;
}; // CalculateScreenSizeInputParam

// ===========================================================================

#define NOFADE_CODE

#define SCREENWIDTH (320) // default screen width in bytes

#define TILEWIDTH (4)

// ===========================================================================

extern int bufferofs; // all drawing is reletive to this

extern bool screenfaded;

// BBi
const int vga_ref_size = 256 * 1'024;
const int vga_plane_count = 4;
const int vga_plane_width = vga_ref_width / 4;
const int vga_plane_height = vga_ref_height + 8;
const int vga_plane_size = vga_plane_width * vga_plane_height;
const int vga_page_size = vga_plane_count * vga_plane_size;
const int vga_page_count = 3;

extern double vga_height_scale; // vga_width / 240
extern double vga_width_scale; // vga_width / 320
extern double vga_wide_scale; // vga_width / 320
extern int vga_width;
extern int vga_height;
extern int vga_area;
extern int vga_3d_view_top_y;
extern int vga_3d_view_bottom_y;

const bool default_vid_widescreen = true;

// Is VSync enabled?
extern bool vid_has_vsync;

// Are we drawing HUD and a level?
extern bool vid_is_hud;

// Are we drawing a level?
extern bool vid_is_3d;

// Is it intro/outro/you-win/etc?
extern bool vid_is_movie;

extern bstone::SpriteCache vid_sprite_cache;

extern double height_compensation_factor;
// BBi

// ===========================================================================

void VL_Startup();

void VL_Shutdown();

void VL_WaitVBL(
	std::uint32_t vbls);

void VL_FillPalette(
	std::uint8_t red,
	std::uint8_t green,
	std::uint8_t blue);

void VL_SetPalette(
	int first,
	int count,
	const std::uint8_t* palette);

void VL_GetPalette(
	int first,
	int count,
	std::uint8_t* palette);

void VL_SetPaletteIntensity(const std::uint8_t* palette, int intensity);

void VL_FadeOut(
	int start,
	int end,
	int red,
	int green,
	int blue,
	int steps);

void VL_FadeIn(
	int start,
	int end,
	const std::uint8_t* palette,
	int steps);

void VL_Plot(
	int x,
	int y,
	std::uint8_t color,
	bool is_transparent = false);

void VL_Hlin(
	int x,
	int y,
	int width,
	std::uint8_t color);

void VL_Vlin(
	int x,
	int y,
	int height,
	std::uint8_t color);

void VL_Bar(
	int x,
	int y,
	int width,
	int height,
	std::uint8_t color,
	bool is_transparent = false);

void VL_MemToLatch(
	const std::uint8_t* source,
	int width,
	int height,
	int dest);

void VL_ScreenToScreen(
	int source,
	int dest,
	int width,
	int height);

void VL_MemToScreen(
	const std::uint8_t* source,
	int width,
	int height,
	int x,
	int y);

void VL_MaskMemToScreen(
	const std::uint8_t* source,
	int width,
	int height,
	int x,
	int y,
	std::uint8_t mask);

void VL_ScreenToMem(
	std::uint8_t* dest,
	int width,
	int height,
	int x,
	int y);

void vid_initialize_cvars(bstone::CVarMgr& cvar_mgr);

void VL_RefreshScreen();

int vl_get_offset(
	int base_offset,
	int x,
	int y);

std::uint8_t vl_get_pixel(
	int base_offset,
	int x,
	int y);

void vl_update_widescreen();

bool vid_cfg_is_ui_stretched() noexcept;
void vid_cfg_set_is_ui_stretched(bool is_enabled);

bool vid_cfg_is_widescreen() noexcept;
void vid_cfg_set_is_widescreen(bool is_enabled);

void vid_set_ui_mask(
	bool value);

void vid_set_ui_mask(
	int x,
	int y,
	int width,
	int height,
	bool value);

void vid_set_ui_mask_3d(
	bool value);

void vid_clear_3d();

void vid_export_ui(
	VgaBuffer& dst_buffer);

void vid_import_ui(
	const VgaBuffer& src_buffer,
	bool is_transparent = false);

void vid_export_ui_mask(
	UiMaskBuffer& dst_buffer);

void vid_import_ui_mask(
	const UiMaskBuffer& src_buffer);

void vid_draw_ui_sprite(
	int sprite_id,
	int center_x,
	int center_y,
	int new_side);

void vid_schedule_take_screenshot();

void vid_schedule_save_screenshot_task(
	int width,
	int height,
	int stride_rgb_888,
	ScreenshotBuffer&& src_pixels_rgb_888,
	bool is_flipped_vertically);

void vid_take_screenshot();

void vid_initialize_vanilla_raycaster();

void vid_initialize_common();

void vid_initialize_ui_buffer();


CalculateScreenSizeInputParam vid_create_screen_size_param() noexcept;

void vid_calculate_window_elements_dimensions(
	const CalculateScreenSizeInputParam& src_param,
	VidLayout& dst_param) noexcept;

void vid_calculate_vga_dimensions() noexcept;

std::string vid_to_string(bool value);


#endif // BSTONE_ID_VL_INCLUDED
