/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

//
// Native Nintendo 3DS software-video interface.
//

#ifndef BSTONE_VIDEO_INCLUDED
#define BSTONE_VIDEO_INCLUDED

#include <memory>

#include "id_vl.h"

namespace bstone
{

class Video
{
public:
	Video() noexcept;

	virtual ~Video();


	virtual StringView get_renderer_name() = 0;

	virtual void clear_vga_buffer() = 0;

	virtual void take_screenshot(
		int width,
		int height,
		int stride_rgb_888,
		ScreenshotBuffer&& src_pixels_rgb_888) = 0;

	virtual void present() = 0;

	virtual void get_palette(
		int offset,
		int count,
		std::uint8_t* vga_palette) const = 0;

	virtual void fill_palette(
		int r,
		int g,
		int b) noexcept = 0;

	virtual void set_palette(
		int offset,
		int count,
		const std::uint8_t* vga_palette) = 0;

	virtual void fade_out(
		int start,
		int end,
		int red,
		int green,
		int blue,
		int steps) = 0;

	virtual void fade_in(
		int start,
		int end,
		const std::uint8_t* palette,
		int steps) = 0;

	virtual void apply_widescreen() = 0;

}; // Video

using VideoUPtr = std::unique_ptr<Video>;

} // bstone

#endif // !BSTONE_VIDEO_INCLUDED
