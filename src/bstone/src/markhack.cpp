/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// Former MARKHACK.ASM
//


#include "3d_def.h"
#include "id_vl.h"


extern int viewwidth;
extern int viewheight;
extern int bufferofs;
extern int centery;
extern double postheight;
extern const std::uint8_t* shadingtable;


enum DrawMode
{
	DRAW_DEFAULT,
	DRAW_LIGHTED
}; // DrawMode



static void generic_draw_post(DrawMode draw_mode)
{
    if (postheight <= 0.0)
    {
        return;
    }

#if defined(__3DS__) || defined(_3DS)

    // New 3DS optimized wall-column renderer.
    // Use fixed-point arithmetic instead of
    // floating-point calculations per pixel.

    if (vid_is_3d)
    {
        const int half_height = viewheight / 2;

        const int count = static_cast<int>(
            std::min(
                postheight,
                static_cast<double>(half_height)));

        if (count <= 0)
        {
            return;
        }

        // 16.16 fixed-point texture coordinates.
        constexpr int fixed_shift = 16;
        constexpr double fixed_scale = 65536.0;

        const auto step = static_cast<std::uint32_t>(
            (32.0 / postheight) * fixed_scale);

        std::uint32_t position = step / 2;

        // Direct pointers to both halves of
        // the wall column in the VGA buffer.
        auto* upper = vga_memory +
            bufferofs +
            (half_height - 1) * vga_width +
            postx;

        auto* lower = upper + vga_width;

        const bool shaded =
            draw_mode == DRAW_LIGHTED;

        for (int h = 0; h < count; ++h)
        {
            const int texel =
                static_cast<int>(
                    position >> fixed_shift);

            const std::uint8_t top_color =
                postsource[31 - texel];

            const std::uint8_t bottom_color =
                postsource[32 + texel];

            if (shaded)
            {
                *upper = shadingtable[top_color];
                *lower = shadingtable[bottom_color];
            }
            else
            {
                *upper = top_color;
                *lower = bottom_color;
            }

            upper -= vga_width;
            lower += vga_width;

            position += step;
        }

        return;
    }

#endif

    int y = posty;


	auto cur_step = 32.0 / postheight;

	auto step = cur_step;
	cur_step /= 2.0;

	int fraction = (vid_is_3d ? vga_width : 1);

	const int max_height = viewheight / 2;

	int screen_column = 0;

	if (vid_is_3d)
	{
		screen_column =
			bufferofs +
			((max_height - 1) * vga_width) +
			postx;
	}
	else
	{
		y += max_height - 1;
	}

	const auto n = static_cast<int>(std::min(postheight, static_cast<double>(max_height)));

	for (int h = 0; h < n; ++h)
	{
		std::uint8_t pixel;
		std::uint8_t pixel_index;

		// top half

		pixel_index = postsource[31 - static_cast<std::ptrdiff_t>(cur_step)];

		if (draw_mode == DRAW_LIGHTED)
		{
			pixel = shadingtable[pixel_index];
		}
		else
		{
			pixel = pixel_index;
		}

		if (vid_is_3d)
		{
			vga_memory[screen_column] = pixel;
		}
		else
		{
			VL_Plot(
				postx,
				y,
				pixel);
		}


		// bottom half

		pixel_index = postsource[32 + static_cast<std::ptrdiff_t>(cur_step)];

		if (draw_mode == DRAW_LIGHTED)
		{
			pixel = shadingtable[pixel_index];
		}
		else
		{
			pixel = pixel_index;
		}

		if (vid_is_3d)
		{
			vga_memory[screen_column + fraction] = pixel;

			screen_column -= vga_width;
			fraction += 2 * vga_width;
		}
		else
		{
			VL_Plot(
				postx,
				y + fraction,
				pixel);

			y -= 1;
			fraction += 2;
		}

		cur_step += step;
	}
}


//
// Draws an unmasked post centered in the viewport
//

void DrawPost()
{
	generic_draw_post(DRAW_DEFAULT);
}


//
// Draws an unmasked light sourced post centered in the viewport
//

void DrawLSPost()
{
	generic_draw_post(DRAW_LIGHTED);
}

// BBi Draws a wall for UI (text presenter, etc.)
void draw_wall_ui(
	const int x,
	const int y,
	const void* raw_wall)
{
	const int wall_side = 64;
	auto wall = static_cast<const std::uint8_t*>(raw_wall);

	for (int w = 0; w < wall_side; ++w)
	{
		const auto wall_base = wall_side * w;

		for (int h = 0; h < wall_side; ++h)
		{
			auto pixel = wall[wall_base + h];

			VL_Plot(
				x + w,
				y + h,
				pixel);
		}
	}
}
// BBi
