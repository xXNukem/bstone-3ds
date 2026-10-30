/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <algorithm>
#include <chrono>

#include "id_ca.h"
#include "id_heads.h"
#include "id_in.h"
#include "id_vh.h"
#include "id_vl.h"

#include "bstone_algorithm.h"
#include "bstone_assert.h"
#include "bstone_atomic_flag.h"
#include "bstone_exception_utils.h"
#include "bstone_file_stream.h"
#include "bstone_fs_utils.h"
#include "bstone_globals.h"
#include "bstone_image_encoder.h"
#include "bstone_logger.h"
#include "bstone_mt_task_mgr.h"
#include "bstone_sprite_cache.h"
#include "bstone_string_helper.h"
#include "bstone_string_view.h"
#include "bstone_sw_video.h"
#include "bstone_text_writer.h"
#include "bstone_time.h"
#include "bstone_version.h"

namespace {

// vid_is_ui_stretched

constexpr auto vid_is_ui_stretched_cvar_name = bstone::StringView{"vid_is_ui_stretched"};
constexpr auto vid_is_ui_stretched_cvar_default = false;

auto vid_is_ui_stretched_cvar = bstone::CVar{
	bstone::CVarBoolTag{},
	vid_is_ui_stretched_cvar_name,
	bstone::CVarFlags::archive,
	vid_is_ui_stretched_cvar_default};

// vid_is_widescreen

constexpr auto vid_is_widescreen_cvar_name = bstone::StringView{"vid_is_widescreen"};
constexpr auto vid_is_widescreen_cvar_default = true;

auto vid_is_widescreen_cvar = bstone::CVar{
	bstone::CVarBoolTag{},
	vid_is_widescreen_cvar_name,
	bstone::CVarFlags::archive,
	vid_is_widescreen_cvar_default};

} // namespace

extern bool is_full_menu_active;

int bufferofs;

bool screenfaded;

std::uint8_t* vga_memory = nullptr;

double vga_height_scale = 0.0;
double vga_width_scale = 0.0;
double vga_wide_scale = 0.0;
int vga_width = 0;
int vga_height = 0;
int vga_area = 0;
int vga_3d_view_top_y = 0;
int vga_3d_view_bottom_y = 0;

bool vid_has_vsync = false;
bool vid_is_hud = false;
bool vid_is_3d = false;
bool vid_is_movie = false;

bstone::SpriteCache vid_sprite_cache;

// BBi
bool vid_is_take_screenshot_scheduled = false;

double height_compensation_factor = 1.2;

namespace
{

void vid_log(
	const std::string& message);

void vid_log_error(
	const std::string& message);

} // namespace

void vid_take_screenshot();

namespace
{

class SaveScreenshotMtTask final :
	public bstone::MtTask
{
public:
	~SaveScreenshotMtTask() override;

	// ----------------------------------------------------------------------
	// MtTask

	void execute() override;

	bool is_completed() const noexcept override;

	void set_completed() override;

	bool is_failed() const noexcept override;

	std::exception_ptr get_exception_ptr() const noexcept override;

	void set_failed(
		std::exception_ptr exception_ptr) override;

	// MtTask
	// ----------------------------------------------------------------------

	void reset(
		int width,
		int height,
		int stride_rgb_888,
		ScreenshotBuffer&& src_pixels_rgb_888,
		bool is_flipped_vertically);

private:
	bstone::AtomicFlag is_completed_{true};
	bstone::AtomicFlag is_failed_{};
	std::exception_ptr exception_ptr_{};

	int width_{};
	int height_{};
	int stride_rgb_888_{};
	ScreenshotBuffer src_pixels_rgb_888_{};
	bool is_flipped_vertically_{};
}; // SaveScreenshotMtTask

SaveScreenshotMtTask::~SaveScreenshotMtTask() = default;

void SaveScreenshotMtTask::execute()
try
{
	const auto date_time = bstone::make_local_date_time();

	const auto date_time_string = bstone::make_local_date_time_string(
		date_time,
		bstone::DateTimeStringFormat::screenshot_file_name
	);

	const auto& screenshot_dir = get_screenshot_dir();

	const auto& assets_info = get_assets_info();

	auto game_string = std::string{};

	if (assets_info.is_aog())
	{
		game_string = "aog";
	}
	else if (assets_info.is_aog_sw())
	{
		game_string = "aog_sw";
	}
	else if (assets_info.is_ps())
	{
		game_string = "ps";
	}

	const auto file_name = "bstone_" + game_string + "_sshot_" + date_time_string + ".png";

	const auto path = bstone::fs_utils::append_path(screenshot_dir, file_name);

	vid_log("Taking screenshot \"" + path + "\".");

	if (is_flipped_vertically_)
	{
		auto row_buffer = std::make_unique<std::uint8_t[]>(stride_rgb_888_);
		auto tmp_row = row_buffer.get();

		const auto half_height = height_ / 2;

		auto src_row = src_pixels_rgb_888_.get();
		auto dst_row = src_row + (stride_rgb_888_ * (height_ - 1));

		for (auto h = 0; h < half_height; ++h)
		{
			std::copy_n(src_row, stride_rgb_888_, tmp_row);
			std::copy_n(dst_row, stride_rgb_888_, src_row);
			std::copy_n(tmp_row, stride_rgb_888_, dst_row);

			src_row += stride_rgb_888_;
			dst_row -= stride_rgb_888_;
		}
	}

	const auto max_dst_buffer_size = stride_rgb_888_ * height_;
	auto dst_buffer = std::make_unique<std::uint8_t[]>(max_dst_buffer_size);
	auto image_encoder = bstone::make_image_encoder(bstone::ImageEncoderType::png);

	auto dst_buffer_size = 0;

	image_encoder->encode_24(
		src_pixels_rgb_888_.get(),
		width_,
		height_,
		dst_buffer.get(),
		max_dst_buffer_size,
		dst_buffer_size
	);

	{
		bstone::FileStream file_stream(
			path.c_str(),
			bstone::file_flags_create | bstone::file_flags_truncate | bstone::file_flags_exclusive);

		file_stream.write_exactly(dst_buffer.get(), dst_buffer_size);
	}
}
catch (...)
{
	vid_log_error("Failed to save a screenshot.");

	const auto messages = bstone::extract_exception_messages();

	for (const auto& message : messages)
	{
		vid_log_error(message);
	}
}

bool SaveScreenshotMtTask::is_completed() const noexcept
{
	return is_completed_.get();
}

void SaveScreenshotMtTask::set_completed()
{
	is_completed_ = true;
}

bool SaveScreenshotMtTask::is_failed() const noexcept
{
	return is_failed_.get();
}

std::exception_ptr SaveScreenshotMtTask::get_exception_ptr() const noexcept
{
	return exception_ptr_;
}

void SaveScreenshotMtTask::set_failed(
	std::exception_ptr exception_ptr)
{
	is_completed_ = true;
	is_failed_ = true;
	exception_ptr_ = exception_ptr;
}

void SaveScreenshotMtTask::reset(
	int width,
	int height,
	int stride_rgb_888,
	ScreenshotBuffer&& src_pixels_rgb_888,
	bool is_flipped_vertically)
{
	is_completed_ = false;
	is_failed_ = false;
	exception_ptr_ = nullptr;

	width_ = width;
	height_ = height;
	stride_rgb_888_ = stride_rgb_888;
	src_pixels_rgb_888_ = std::move(src_pixels_rgb_888);
	is_flipped_vertically_ = is_flipped_vertically;
}

constexpr auto max_screenshot_tasks = 16;

using SaveScreenshotMtTasks = std::array<SaveScreenshotMtTask, max_screenshot_tasks>;

SaveScreenshotMtTasks vid_save_screenshot_mt_tasks{};

// --------------------------------------------------------------------------

/*

Window elements.

A--B------------------------------------------------------------------------C
|                                                                           |
|                                                                           |
D  E------------F-----------------------------------------G--------------H  |
|  | FILLER (1) |             TOP STATUS BAR              |  FILLER (1)  |  |
|  I- - - - - - +-----------------------------------------+ - - - - - - -+  |
|  J- - - - - - +-----------------------------------------+ - - - - - - -+  |
|  |            |                                         |              |  |
|  |   FILLER   |                                         |    FILLER    |  |
|  |     OR     |                 3D VIEW                 |      OR      |  |
|  |  3D VIEW   |                                         |   3D VIEW*   |  |
|  |     (2)    |                                         |      (2)     |  |
|  K- - - - - - +-----------------------------------------+ - - - - - - -+  |
|  L- - - - - - +-----------------------------------------+ - - - - - - -+  |
|  |            |                                         |              |  |
|  |   FILLER   |            BOTTOM STATUS BAR            |    FILLER    |  |
|  |     (3)    |                                         |      (3)     |  |
|  |            |                                         |              |  |
|  M------------+-----------------------------------------+--------------+  |
|                                                                           |
|                                                                           |
N---------------------------------------------------------------------------+

Legend:
	- AC - window width;
	- AN - window height;
	- AB - window viewport left width;
	- AD - window viewport top height;
	- EH - screen width;
	- EM - screen height;
	- EF - screen left filler width;
	- GH - screen right filler width;
	- EI - screen top filler height;
	- LM - screen bottom filler height;
	- FG - screen width (4x3);
	- EJ - screen viewport top height;
	- KM - screen viewport bottom height;
	- JK - screen viewport height;
	- IJ/KL - margin height.

Notes:
	(1) Top status bar if stretch is on or filler otherwise.
	(2) 3D view if widescreen is on or filler otherwise.
	(3) Bottom status bar if stretch is on or filler otherwise.

*/

} // namespace

VgaBuffer vid_ui_buffer_;
UiMaskBuffer vid_mask_buffer_;

VidLayout vid_layout_;

namespace
{

} // namespace

void vid_calculate_window_elements_dimensions(
	const CalculateScreenSizeInputParam& src_param,
	VidLayout& dst_param) noexcept
{
	dst_param.width = src_param.width;
	dst_param.height = src_param.height;

	const auto window_width = src_param.window_width;
	const auto window_height = src_param.window_height;

	BSTONE_ASSERT(window_width >= vga_ref_width);
	BSTONE_ASSERT(window_height >= vga_ref_height_4x3);

	auto screen_width = (window_width / 2) * 2;
	auto screen_height = (window_height / 2) * 2;

	const auto min_r_ratio = 3.0 / 4.0;
	const auto r_ratio = static_cast<double>(window_height) / static_cast<double>(window_width);

	if (r_ratio > min_r_ratio)
	{
		screen_height = (((screen_width * 3) / 4) / 2) * 2;
	}

	//
	const auto window_viewport_left_width = (window_width - screen_width) / 2;
	const auto window_viewport_right_width = window_width - screen_width - window_viewport_left_width;
	const auto window_viewport_top_height = (window_height - screen_height) / 2;
	const auto window_viewport_bottom_height = window_height - screen_height - window_viewport_top_height;

	//
	const auto screen_width_4x3 = (((screen_height * 4) / 3) / 2) * 2;

	const auto screen_left_filler_width = (screen_width - screen_width_4x3) / 2;
	const auto screen_right_filler_width = screen_width - screen_width_4x3 - screen_left_filler_width;

	const auto screen_top_filler_height = (ref_top_bar_height * screen_height) / vga_ref_height;
	const auto screen_bottom_filler_height = (ref_bottom_bar_height * screen_height) / vga_ref_height;

	auto screen_viewport_left_width = 0;
	auto screen_viewport_right_width = 0;
	auto screen_viewport_width = 0;

	if (src_param.is_widescreen)
	{
		screen_viewport_width = screen_width;
	}
	else
	{
		screen_viewport_left_width = screen_left_filler_width;
		screen_viewport_right_width = screen_width - screen_width_4x3 - screen_left_filler_width;
		screen_viewport_width = screen_width_4x3;
	}

	const auto screen_viewport_top_height = ((ref_top_bar_height + ref_3d_margin) * screen_height) / vga_ref_height;
	const auto screen_viewport_bottom_height = ((ref_bottom_bar_height + ref_3d_margin) * screen_height) / vga_ref_height;
	const auto screen_viewport_height = screen_height - screen_viewport_top_height - screen_viewport_bottom_height;

	//
	dst_param.window_width = window_width;
	dst_param.window_height = window_height;

	dst_param.window_viewport_left_width = window_viewport_left_width;
	dst_param.window_viewport_right_width = window_viewport_right_width;
	dst_param.window_viewport_top_height = window_viewport_top_height;
	dst_param.window_viewport_bottom_height = window_viewport_bottom_height;

	dst_param.screen_width = screen_width;
	dst_param.screen_height = screen_height;

	dst_param.screen_width_4x3 = screen_width_4x3;

	dst_param.screen_left_filler_width = screen_left_filler_width;
	dst_param.screen_right_filler_width = screen_right_filler_width;

	dst_param.screen_top_filler_height = screen_top_filler_height;
	dst_param.screen_bottom_filler_height = screen_bottom_filler_height;

	dst_param.screen_viewport_left_width = screen_viewport_left_width;
	dst_param.screen_viewport_right_width = screen_viewport_right_width;
	dst_param.screen_viewport_width = screen_viewport_width;

	dst_param.screen_viewport_top_height = screen_viewport_top_height;
	dst_param.screen_viewport_bottom_height = screen_viewport_bottom_height;
	dst_param.screen_viewport_height = screen_viewport_height;
}

CalculateScreenSizeInputParam vid_create_screen_size_param() noexcept
{
	auto result = CalculateScreenSizeInputParam{};
	result.is_widescreen = vid_cfg_is_widescreen();
	result.width = vid_native_width;
	result.height = vid_native_height;
	result.window_width = vid_native_width;
	result.window_height = vid_native_height;
	return result;
}

void vid_calculate_vga_dimensions() noexcept
{
	vga_width = vid_layout_.screen_viewport_width;

	vga_height = vid_layout_.screen_height;

	vga_width_scale = static_cast<double>(vga_width) / static_cast<double>(vga_ref_width);
	vga_height_scale = static_cast<double>(vga_height) / static_cast<double>(vga_ref_height);

	vga_wide_scale =
		static_cast<double>(vga_ref_height * vga_width) /
		static_cast<double>(vga_ref_width * vga_height);

	vga_area = vga_width * vga_height;
}

void vid_initialize_cvars(bstone::CVarMgr& cvar_mgr)
{
	cvar_mgr.add(vid_is_ui_stretched_cvar);
	cvar_mgr.add(vid_is_widescreen_cvar);
}

namespace
{

const std::string& vid_get_vid_string()
{
	static const auto result = std::string{"[VID]"};

	return result;
}

void vid_log()
{
	bstone::globals::logger->log_information();
}

void vid_log(
	const std::string& message)
{
	bstone::globals::logger->log(
		bstone::LoggerMessageType::information,
		(vid_get_vid_string() + ' ' + message).c_str()
	);
}

void vid_log_error(
	const std::string& message)
{
	bstone::globals::logger->log(
		bstone::LoggerMessageType::error,
		(vid_get_vid_string() + ' ' + message).c_str()
	);
}

} // namespace

std::string vid_to_string(bool value)
{
	return value ? "true" : "false";
}

void vid_initialize_vanilla_raycaster()
try {
	SetupWalls();
	NewViewSize();
	SetPlaneViewSize();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

namespace
{

void vid_log_common_configuration()
{
	vid_log();
	vid_log("--------------------");
	vid_log("Common configuration");
	vid_log("--------------------");

	vid_log("UI stretched: " + vid_to_string(vid_cfg_is_ui_stretched()));
	vid_log("Widescreen: " + vid_to_string(vid_cfg_is_widescreen()));

	vid_log("--------------------");
}

} // namespace

void vid_initialize_common()
try {

	vid_log_common_configuration();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void vid_initialize_ui_buffer()
try {
	const auto area = vga_ref_width * vga_ref_height;

	vid_ui_buffer_.resize(area);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

namespace
{

auto g_video = bstone::VideoUPtr{};

} // namespace

void vid_schedule_save_screenshot_task(
	int width,
	int height,
	int stride_rgb_888,
	ScreenshotBuffer&& src_pixels_rgb_888,
	bool is_flipped_vertically)
try {
	for (auto& task : vid_save_screenshot_mt_tasks)
	{
		if (task.is_completed())
		{
			task.reset(
				width,
				height,
				stride_rgb_888,
				std::move(src_pixels_rgb_888),
				is_flipped_vertically
			);

			bstone::MtTaskPtr tasks_ptr[] = {&task};

			mt_task_manager_->add_tasks(tasks_ptr, 1);

			return;
		}
	}

	vid_log_error("No more screenshot tasks available.");
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void vid_take_screenshot()
try
{
	vid_is_take_screenshot_scheduled = false;

	const auto width = vid_native_width;
	const auto height = vid_native_height;
	const auto stride_rgb_888 = (((3 * width) + 3) / 4) * 4;
	auto src_rgb_888_pixels = std::make_unique<std::uint8_t[]>(stride_rgb_888 * height);

	g_video->take_screenshot(
		width,
		height,
		stride_rgb_888,
		std::move(src_rgb_888_pixels)
	);
}
catch (const std::exception& ex)
{
	vid_log_error("Failed to take a screenshot.");
	vid_log_error(ex.what());
}

namespace
{

} // namespace
// BBi

// ===========================================================================

// asm

void VL_WaitVBL(
	std::uint32_t vbls)
{
	if (vbls == 0)
	{
		return;
	}

	sys_sleep_for(1000 * vbls / TickBase);
}

// ===========================================================================

// BBi Moved from jm_free.cpp
void VL_Startup()
try {
	g_video = nullptr;

	g_video = bstone::make_sw_video();
	const auto name = g_video->get_renderer_name();
	vid_log(std::string{name.get_data(), static_cast<std::size_t>(name.get_size())});

	VL_SetPalette(0, 255, vgapal);

	in_handle_events();

} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED
// BBi

void VL_Shutdown()
{
	g_video = nullptr;
}

// ===========================================================================

/*
=============================================================================

								PALETTE OPS

				To avoid snow, do a WaitVBL BEFORE calling these

=============================================================================
*/

void VL_FillPalette(
	std::uint8_t red,
	std::uint8_t green,
	std::uint8_t blue)
try {
	g_video->fill_palette(red, green, blue);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void VL_SetPalette(
	int first,
	int count,
	const std::uint8_t* palette)
try {
	g_video->set_palette(first, count, palette);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void VL_GetPalette(
	int first,
	int count,
	std::uint8_t* palette)
try {
	g_video->get_palette(first, count, palette);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Fades the current palette to the given color in the given number of steps.
void VL_FadeOut(
	int start,
	int end,
	int red,
	int green,
	int blue,
	int steps)
try {
	BSTONE_ASSERT(start >= 0);
	BSTONE_ASSERT(end >= 0);
	BSTONE_ASSERT(red >= 0 && red <= 0xFF);
	BSTONE_ASSERT(green >= 0 && green <= 0xFF);
	BSTONE_ASSERT(blue >= 0 && blue <= 0xFF);
	BSTONE_ASSERT(steps > 0);
	BSTONE_ASSERT(start <= end);

	g_video->fade_out(start, end, red, green, blue, steps);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void VL_FadeIn(
	int start,
	int end,
	const std::uint8_t* palette,
	int steps)
try {
	BSTONE_ASSERT(start >= 0);
	BSTONE_ASSERT(end >= 0);
	BSTONE_ASSERT(palette != nullptr);
	BSTONE_ASSERT(steps > 0);
	BSTONE_ASSERT(start <= end);

	g_video->fade_in(start, end, palette, steps);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void VL_SetPaletteIntensity(const std::uint8_t* palette, int intensity)
try {
	auto new_palette = VgaPalette{};

	auto cmap = &new_palette[0][0];

	intensity = 63 - intensity;

	for (auto i = 0; i < 256; ++i)
	{
		const auto red = std::max(palette[0] - intensity, 0);
		const auto green = std::max(palette[1] - intensity, 0);
		const auto blue = std::max(palette[2] - intensity, 0);

		cmap[0] = static_cast<std::uint8_t>(red);
		cmap[1] = static_cast<std::uint8_t>(green);
		cmap[2] = static_cast<std::uint8_t>(blue);

		palette += 3;
		cmap += 3;
	}

	VL_SetPalette(0, 256, &new_palette[0][0]);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

/*
=============================================================================

 PIXEL OPS

=============================================================================
*/

void VL_Plot(
	int x,
	int y,
	std::uint8_t color,
	bool is_transparent)
{
	const auto offset = (y * vga_ref_width) + x;

	vid_ui_buffer_[offset] = color;
	vid_mask_buffer_[offset] = !is_transparent;
}

void VL_Hlin(
	int x,
	int y,
	int width,
	std::uint8_t color)
{
	VL_Bar(x, y, width, 1, color);
}

void VL_Vlin(
	int x,
	int y,
	int height,
	std::uint8_t color)
{
	VL_Bar(x, y, 1, height, color);
}

void VL_Bar(
	int x,
	int y,
	int width,
	int height,
	std::uint8_t color,
	bool is_transparent)
{
	if (x == 0 && width == vga_ref_width)
	{
		const auto offset = y * vga_ref_width;
		const auto count = height * vga_ref_width;

		std::fill(
			vid_ui_buffer_.begin() + offset,
			vid_ui_buffer_.begin() + offset + count,
			color);

		std::fill(
			vid_mask_buffer_.begin() + offset,
			vid_mask_buffer_.begin() + offset + count,
			!is_transparent);
	}
	else
	{
		for (int i = 0; i < height; ++i)
		{
			const auto offset = ((y + i) * vga_ref_width) + x;

			std::fill(
				vid_ui_buffer_.begin() + offset,
				vid_ui_buffer_.begin() + offset + width,
				color);

			std::fill(
				vid_mask_buffer_.begin() + offset,
				vid_mask_buffer_.begin() + offset + width,
				!is_transparent);
		}
	}
}

/*
============================================================================

 MEMORY OPS

============================================================================
*/

void VL_MemToLatch(
	const std::uint8_t* source,
	int width,
	int height,
	int dest)
{
	for (int p = 0; p < 4; ++p)
	{
		for (int h = 0; h < height; ++h)
		{
			for (int w = p; w < width; w += 4)
			{
				const auto pixel = *source++;
				const auto offset = dest + ((h * width) + w);

				latches_cache[offset] = pixel;
			}
		}
	}
}

void VL_MemToScreen(
	const std::uint8_t* source,
	int width,
	int height,
	int x,
	int y)
{
	for (int p = 0; p < 4; ++p)
	{
		for (int h = 0; h < height; ++h)
		{
			for (int w = p; w < width; w += 4)
			{
				VL_Plot(x + w, y + h, *source++);
			}
		}
	}
}

void VL_MaskMemToScreen(
	const std::uint8_t* source,
	int width,
	int height,
	int x,
	int y,
	std::uint8_t mask)
{
	for (int p = 0; p < 4; ++p)
	{
		for (int h = 0; h < height; ++h)
		{
			for (int w = p; w < width; w += 4)
			{
				const auto color = *source++;

				if (color != mask)
				{
					VL_Plot(x + w, y + h, color);
				}
			}
		}
	}
}

void VL_ScreenToMem(
	std::uint8_t* dest,
	int width,
	int height,
	int x,
	int y)
{
	for (int p = 0; p < 4; ++p)
	{
		for (int h = 0; h < height; ++h)
		{
			for (int w = p; w < width; w += 4)
			{
				*dest++ = vl_get_pixel(bufferofs, x + w, y + h);
			}
		}
	}
}

void VL_LatchToScreen(
	int source,
	int width,
	int height,
	int x,
	int y)
{
	for (int h = 0; h < height; ++h)
	{
		const auto src_offset = source + (h * width);
		const auto dst_offset = (vga_ref_width * (y + h)) + x;

		std::copy(
			latches_cache.cbegin() + src_offset,
			latches_cache.cbegin() + src_offset + width,
			vid_ui_buffer_.begin() + dst_offset);

		std::fill(
			vid_mask_buffer_.begin() + dst_offset,
			vid_mask_buffer_.begin() + dst_offset + width,
			true);
	}
}

void VL_ScreenToScreen(
	int source,
	int dest,
	int width,
	int height)
{
	for (int h = 0; h < height; ++h)
	{
		const auto src_offset = source + (h * vga_ref_width);
		const auto dst_offset = dest + (h * vga_ref_width);

		std::copy(
			vid_ui_buffer_.cbegin() + src_offset,
			vid_ui_buffer_.cbegin() + src_offset + width,
			vid_ui_buffer_.begin() + dst_offset);

		std::fill(
			vid_mask_buffer_.begin() + dst_offset,
			vid_mask_buffer_.begin() + dst_offset + width,
			true);
	}
}

void JM_VGALinearFill(
	int start,
	int length,
	std::uint8_t fill)
{
	std::fill(
		vid_ui_buffer_.begin() + start,
		vid_ui_buffer_.begin() + start + length,
		fill);

	std::fill(
		vid_mask_buffer_.begin() + start,
		vid_mask_buffer_.begin() + start + length,
		true);
}

void VL_RefreshScreen()
try {
	g_video->present();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

int vl_get_offset(
	int base_offset,
	int x,
	int y)
{
	return base_offset + (y * vga_width) + x;
}

std::uint8_t vl_get_pixel(
	int base_offset,
	int x,
	int y)
{
	static_cast<void>(base_offset);

	return vid_ui_buffer_[(y * vga_ref_width) + x];
}

void vl_update_widescreen()
try {
	g_video->apply_widescreen();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

bool vid_cfg_is_ui_stretched() noexcept
{
	return vid_is_ui_stretched_cvar.get_bool();
}

void vid_cfg_set_is_ui_stretched(bool is_enabled)
{
	vid_is_ui_stretched_cvar.set_bool(is_enabled);
}

bool vid_cfg_is_widescreen() noexcept
{
	return vid_is_widescreen_cvar.get_bool();
}

void vid_cfg_set_is_widescreen(bool is_enabled)
{
	vid_is_widescreen_cvar.set_bool(is_enabled);
}

void vid_set_ui_mask(
	bool value)
{
	vid_mask_buffer_.fill(
		value);
}

void vid_set_ui_mask(
	int x,
	int y,
	int width,
	int height,
	bool value)
{
	for (int h = 0; h < height; ++h)
	{
		const auto offset = ((y + h) * vga_ref_width) + x;

		std::fill(
			vid_mask_buffer_.begin() + offset,
			vid_mask_buffer_.begin() + offset + width,
			value);
	}
}

void vid_set_ui_mask_3d(
	bool value)
{
	vid_set_ui_mask(
		0,
		ref_3d_view_top_y - ref_3d_margin,
		vga_ref_width,
		ref_3d_view_height + 2 * ref_3d_margin,
		value);
}

void vid_clear_3d()
try {
	g_video->clear_vga_buffer();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void vid_export_ui(
	VgaBuffer& dst_buffer)
{
	dst_buffer = vid_ui_buffer_;
}

void vid_import_ui(
	const VgaBuffer& src_buffer,
	bool is_transparent)
{
	vid_ui_buffer_ = src_buffer;
	vid_set_ui_mask(!is_transparent);
}

void vid_export_ui_mask(
	UiMaskBuffer& dst_buffer)
{
	dst_buffer = vid_mask_buffer_;
}

void vid_import_ui_mask(
	const UiMaskBuffer& src_buffer)
{
	vid_mask_buffer_ = src_buffer;
}

void vid_draw_ui_sprite(
	int sprite_id,
	int center_x,
	int center_y,
	int new_side)
try {
	constexpr auto dimension = bstone::Sprite::dimension;

	const auto sprite_ptr = vid_sprite_cache.cache(
		sprite_id);

	const auto sprite_width = sprite_ptr->get_width();
	const auto sprite_height = sprite_ptr->get_height();

	const auto left = sprite_ptr->get_left();
	const auto x1 = center_x + ((new_side * (left - (dimension / 2))) / dimension);
	const auto x2 = x1 + ((sprite_width * new_side) / dimension);

	const auto top = sprite_ptr->get_top();
	const auto y1 = center_y + ((new_side * (top - (dimension / 2))) / dimension) - 2;
	const auto y2 = y1 + ((sprite_height * new_side) / dimension);

	for (int x = x1; x < x2; ++x)
	{
		if (x < 0)
		{
			continue;
		}

		if (x >= vga_ref_width)
		{
			break;
		}

		const auto column_index = ((sprite_width - 1) * (x - x1)) / (x2 - x1 - 1);
		const auto column = sprite_ptr->get_column(column_index);

		for (int y = y1; y < y2; ++y)
		{
			if (y < 0)
			{
				continue;
			}

			if (y >= vga_ref_height)
			{
				break;
			}

			const auto row_index = ((sprite_height - 1) * (y - y1)) / (y2 - y1 - 1);
			const auto sprite_color = column[row_index];

			if (sprite_color < 0)
			{
				continue;
			}

			const auto color_index = static_cast<std::uint8_t>(sprite_color);

			VL_Plot(x, y, color_index);
		}
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void vid_schedule_take_screenshot()
{
	vid_is_take_screenshot_scheduled = true;
}

// BBi
