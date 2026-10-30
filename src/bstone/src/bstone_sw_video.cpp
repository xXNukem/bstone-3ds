/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// Software raycasting with native Citro3D/Citro2D presentation.

#include <algorithm>
#include <utility>
#include <chrono>
#include <cstdio>
#include "3d_def.h"
#include "id_in.h"
#include "id_vh.h"
#include "id_vl.h"

#include "bstone_assert.h"
#include "bstone_exception.h"
#include "bstone_logger.h"
#include "bstone_single_pool_resource.h"
#include "bstone_span.h"
#include "bstone_sw_video.h"
#include "bstone_video.h"
#include <atomic>
#include <array>

#include <3ds.h>
#include <citro2d.h>
#include "bstone_3ds_hud.h"
#include "bstone_3ds_intro.h"

extern bool n3ds_stereo_active;
namespace bstone {

namespace {

// Independent conversion work for each stereoscopic eye.
// The worker only writes CPU-accessible texture memory.
// All Citro3D operations remain on the main thread.
struct N3dsScreenConversion
{
    u16* destination = nullptr;
    const u8* source = nullptr;
    const u16* palette = nullptr;
    const u32* swizzle_x = nullptr;
    const u32* swizzle_y = nullptr;
    int source_base = 0;
    int source_stride = 0;
    int width = 0;
    int height = 0;
};

void n3ds_convert_screen(void* context)
{
    const auto& job = *static_cast<const N3dsScreenConversion*>(context);

    for (int y = 0; y < job.height; ++y)
    {
        const auto* source_row = job.source +
            job.source_base + y * job.source_stride;
        auto* destination_row = job.destination + job.swizzle_y[y];

        for (int x = 0; x < job.width; ++x)
        {
            destination_row[job.swizzle_x[x]] =
                job.palette[source_row[x]];
        }
    }
}

class N3dsEyeWorker
{
public:
    struct Job
    {
        void (*callback)(void*) = nullptr;
        void* context = nullptr;
    };

    void initialize()
    {
        if (attempted_)
            return;

        attempted_ = true;

        LightEvent_Init(&start_event_, RESET_ONESHOT);
        LightEvent_Init(&finish_event_, RESET_ONESHOT);

        thread_ = threadCreate(
            &N3dsEyeWorker::thread_main,
            this,
            0x4000,
            0x30,
            2,
            false
        );
    }

    bool available() const noexcept
    {
        return thread_ != nullptr;
    }

    void submit(Job& job)
    {
        pending_job_.store(
            &job,
            std::memory_order_release
        );

        LightEvent_Signal(&start_event_);
    }

    void wait()
    {
        LightEvent_Wait(&finish_event_);

        pending_job_.store(
            nullptr,
            std::memory_order_release
        );
    }

    void shutdown()
    {
        if (!thread_)
            return;

        stopping_.store(
            true,
            std::memory_order_release
        );

        LightEvent_Signal(&start_event_);

        threadJoin(thread_, U64_MAX);
        threadFree(thread_);

        thread_ = nullptr;
    }

private:
    static void thread_main(void* argument)
    {
        auto* worker =
            static_cast<N3dsEyeWorker*>(argument);

        for (;;)
        {
            LightEvent_Wait(&worker->start_event_);

            if (worker->stopping_.load(
                std::memory_order_acquire))
            {
                break;
            }

            auto* job = worker->pending_job_.load(
                std::memory_order_acquire
            );

            if (job && job->callback)
            {
                job->callback(job->context);
            }

            LightEvent_Signal(&worker->finish_event_);
        }
    }

    Thread thread_ = nullptr;

    LightEvent start_event_{};
    LightEvent finish_event_{};

    std::atomic<Job*> pending_job_{nullptr};
    std::atomic<bool> stopping_{false};

    bool attempted_ = false;
};

class SwVideo final : public Video
{
public:
	SwVideo();

	~SwVideo() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

	StringView get_renderer_name() override;
	void clear_vga_buffer() override;
	void take_screenshot(
		int width,
		int height,
		int stride_rgb_888,
		ScreenshotBuffer&& src_pixels_rgb_888) override;
	void present() override;

	void get_palette(int offset, int count, std::uint8_t* vga_palette) const override;
	void fill_palette(int r, int g, int b) noexcept override;
	void set_palette(int offset, int count, const std::uint8_t* vga_palette) override;

	void apply_widescreen() override;

	void fade_out(int start, int end, int red, int green, int blue, int steps) override;
	void fade_in(int start, int end, const std::uint8_t* palette, int steps) override;

private:
	static constexpr auto log_prefix = "[VIDSW] ";

private:
	void initialize_video();
    void uninitialize_gpu() noexcept;
	void initialize_vga_buffer();
	void initialize_palette();
	void calculate_dimensions();
	void uninitialize_vga_buffer() noexcept;
	void update_palette_from_vga(int offset, int count);

private:
    bool citro3d_initialized_ = false;
    bool citro2d_initialized_ = false;
    C3D_Tex gpu_screen_texture{};
    C3D_Tex gpu_screen_texture_right{};
    C3D_Tex gpu_ui_texture{};
    bool gpu_textures_ready = false;
    bool gpu_screen_format_ready = false;
	VgaBuffer sw_vga_buffer_{};
	VgaPalette vga_palette_{};
	PackedPalette palette_{};

	C3D_RenderTarget* gpu_top_{};
	C3D_RenderTarget* gpu_top_right_{};
	C3D_RenderTarget* gpu_bottom_{};

	N3dsEyeWorker eye_worker_{};

};

// ==========================================================================

using SwVideoPool = bstone::SinglePoolResource<SwVideo>;
SwVideoPool sw_video_pool{};

// ==========================================================================

SwVideo::SwVideo()
{
    try
    {
        initialize_video();
    }
    catch (...)
    {
        uninitialize_gpu();
        uninitialize_vga_buffer();
        throw;
    }
}

SwVideo::~SwVideo()
{
    eye_worker_.shutdown();

    uninitialize_gpu();
    uninitialize_vga_buffer();
}

void* SwVideo::operator new(std::size_t size)
try {
	return sw_video_pool.allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::operator delete(void* ptr)
{
	sw_video_pool.deallocate(ptr);
}

StringView SwVideo::get_renderer_name()
{
	return "3DS software / Citro3D";
}

void SwVideo::clear_vga_buffer()
{
	std::fill(sw_vga_buffer_.begin(), sw_vga_buffer_.end(), VgaBuffer::value_type{});
}

void SwVideo::apply_widescreen()
try {

	uninitialize_vga_buffer();
	calculate_dimensions();
	initialize_vga_buffer();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::take_screenshot(
	int width,
	int height,
	int stride_rgb_888,
	ScreenshotBuffer&& src_pixels_rgb_888)
try {
    C3D_FrameSync();
    u16 framebuffer_width = 0;
    u16 framebuffer_height = 0;
    const auto* framebuffer = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, &framebuffer_width, &framebuffer_height);
    if (!framebuffer || width != framebuffer_height || height != framebuffer_width ||
        gfxGetScreenFormat(GFX_TOP) != GSP_BGR8_OES)
    {
        BSTONE_THROW_STATIC_SOURCE("Unexpected Nintendo 3DS screenshot framebuffer.");
    }
    for (int y = 0; y < height; ++y)
    {
        auto* destination = src_pixels_rgb_888.get() + y * stride_rgb_888;
        for (int x = 0; x < width; ++x)
        {
            const auto* source = framebuffer + 3 * (x * height + height - 1 - y);
            destination[3 * x] = source[2];
            destination[3 * x + 1] = source[1];
            destination[3 * x + 2] = source[0];
        }
    }

	vid_schedule_save_screenshot_task(
		width,
		height,
		stride_rgb_888,
		std::move(src_pixels_rgb_888),
		false);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::present()
try {
	

		constexpr int texture_width = 512;
		constexpr int texture_height = 256;

        // Cached Morton/swizzled texture offsets.
        // Calculated only once instead of once per pixel.

        static u32 swizzle_x[texture_width]{};
        static u32 swizzle_y[texture_height]{};
        static bool swizzle_ready = false;

        if (!swizzle_ready)
        {
            for (int x = 0; x < texture_width; ++x)
            {
                swizzle_x[x] =
                    ((x >> 3) * 64) |
                    (x & 1) |
                    ((x & 2) << 1) |
                    ((x & 4) << 2);
            }

            for (int y = 0; y < texture_height; ++y)
            {
                swizzle_y[y] =
                    ((y >> 3) * (texture_width >> 3) * 64) |
                    ((y & 1) << 1) |
                    ((y & 2) << 2) |
                    ((y & 4) << 3);
            }

            swizzle_ready = true;
        }

		// Comprobar que ambas imágenes caben en las texturas.
		if (vga_width > texture_width ||
			vga_height > texture_height ||
			vga_ref_width > texture_width ||
			vga_ref_height > texture_height)
		{
			BSTONE_THROW_STATIC_SOURCE(
				"3DS GPU texture dimensions exceeded.");
		}

		// Crear las tres texturas una sola vez.
		if (!gpu_textures_ready)
		{
			if (!C3D_TexInit(
				&gpu_screen_texture,
				texture_width,
				texture_height,
				GPU_RGB565))
			{
				BSTONE_THROW_STATIC_SOURCE(
					"Failed to create left eye GPU texture.");
			}

			if (!C3D_TexInit(
				&gpu_screen_texture_right,
				texture_width,
				texture_height,
				GPU_RGB565))
			{
				C3D_TexDelete(&gpu_screen_texture);

				BSTONE_THROW_STATIC_SOURCE(
					"Failed to create right eye GPU texture.");
			}

			if (!C3D_TexInit(
				&gpu_ui_texture,
				texture_width,
				texture_height,
				GPU_RGBA8))
			{
				C3D_TexDelete(&gpu_screen_texture_right);
				C3D_TexDelete(&gpu_screen_texture);

				BSTONE_THROW_STATIC_SOURCE(
					"Failed to create UI GPU texture.");
			}

			C3D_Tex* textures[] =
			{
				&gpu_screen_texture,
				&gpu_screen_texture_right,
				&gpu_ui_texture
			};

			for (auto* texture : textures)
			{
				C3D_TexSetFilter(
					texture,
					GPU_NEAREST,
					GPU_NEAREST);

				C3D_TexSetWrap(
					texture,
					GPU_CLAMP_TO_EDGE,
					GPU_CLAMP_TO_EDGE);
			}

			gpu_textures_ready = true;
		}

		// Usar el formato de pantalla que ya probamos.
		if (!gpu_screen_format_ready)
		{
			gfxSetScreenFormat(GFX_TOP, GSP_BGR8_OES);
			gpu_screen_format_ready = true;
		}

		
        // Temporary performance profiler.
        using ProfileClock = std::chrono::steady_clock;

        static double profile_sums[4]{};
		static double profile_pixels_sum = 0.0;
		static double profile_flush_sum = 0.0;

        static int profile_samples = 0;
        static bool profile_stereo = false;

        static char profile_line1[48] = "MEASURING...";
        static char profile_line2[48] = "";

        ProfileClock::time_point profile_marks[5];
		double profile_pixels_frame = 0.0;
		double profile_flush_frame = 0.0;

        profile_marks[0] = ProfileClock::now();

        C3D_FrameBegin(0);

        profile_marks[1] = ProfileClock::now();

		// ---------------------------------------------
		// 1. Convertir ambos ojos del escenario a texturas GPU.
		// ---------------------------------------------

		const bool stereo_active =
			vid_is_hud && ::n3ds_stereo_active;

		
        if (vid_is_hud)
        {
            const int eye_count = stereo_active ? 2 : 1;

            // Build RGB565 lookup only when the VGA palette changes.
            static std::array<u16, 256> palette565{};
            static PackedPalette palette565_source{};
            static bool palette565_valid = false;

            if (!palette565_valid || palette565_source != palette_)
            {
                for (std::size_t i = 0; i < palette565.size(); ++i)
                {
                    const u32 color = palette_[i];
                    palette565[i] = static_cast<u16>(
                        ((color >> 8) & 0xF800U) |
                        ((color >> 5) & 0x07E0U) |
                        ((color >> 3) & 0x001FU));
                }

                palette565_source = palette_;
                palette565_valid = true;
            }

            if (stereo_active)
            {
                eye_worker_.initialize();
            }

            N3dsScreenConversion left_job{};
            left_job.destination = static_cast<u16*>(gpu_screen_texture.data);
            left_job.source = sw_vga_buffer_.data();
            left_job.palette = palette565.data();
            left_job.swizzle_x = swizzle_x;
            left_job.swizzle_y = swizzle_y;
            left_job.source_base = 0;
            left_job.source_stride = vga_width;
            left_job.width = vga_width;
            left_job.height = vga_height;

            const auto pixels_start = ProfileClock::now();

            if (eye_count == 2)
            {
                N3dsScreenConversion right_job = left_job;
                right_job.destination =
                    static_cast<u16*>(gpu_screen_texture_right.data);
                right_job.source_base = vga_area;

                if (eye_worker_.available())
                {
                    N3dsEyeWorker::Job worker_job{};
                    worker_job.callback = &n3ds_convert_screen;
                    worker_job.context = &right_job;

                    // Core 2 converts the right eye in parallel.
                    eye_worker_.submit(worker_job);
                    n3ds_convert_screen(&left_job);
                    eye_worker_.wait();
                }
                else
                {
                    // Core-2 access is not guaranteed in every launcher.
                    n3ds_convert_screen(&left_job);
                    n3ds_convert_screen(&right_job);
                }
            }
            else
            {
                n3ds_convert_screen(&left_job);
            }

            const auto pixels_end = ProfileClock::now();

            // Flush textures only after both CPU conversions finish.
            // Citro3D remains exclusively on the main thread.
            C3D_TexFlush(&gpu_screen_texture);

            if (eye_count == 2)
            {
                C3D_TexFlush(&gpu_screen_texture_right);
            }

            const auto flush_end = ProfileClock::now();

            profile_pixels_frame +=
                std::chrono::duration<double, std::milli>(
                    pixels_end - pixels_start).count();

            profile_flush_frame +=
                std::chrono::duration<double, std::milli>(
                    flush_end - pixels_end).count();
        }

		profile_marks[2] = ProfileClock::now();
		// ---------------------------------------------
		// 2. Convertir la interfaz a textura GPU.
		// ---------------------------------------------

        // Cache the top-screen UI texture when unchanged.
        static VgaBuffer previous_ui;
        static UiMaskBuffer previous_mask{};
        static PackedPalette previous_palette{};
        static bool previous_hud = false;
        static bool ui_cache_valid = false;

        bool ui_dirty =
            !ui_cache_valid ||
            previous_hud != vid_is_hud ||
            previous_palette != palette_ ||
            previous_mask != vid_mask_buffer_ ||
            previous_ui.size() != vid_ui_buffer_.size();

        if (!ui_dirty)
        {
            if (vid_is_hud)
            {
                // Only visible pixels matter during gameplay.
                for (int y = 0;
                     y < vga_ref_height && !ui_dirty;
                     ++y)
                {
                    if (y < ref_3d_view_top_y ||
                        y > ref_3d_view_bottom_y)
                        continue;

                    for (int x = 0; x < vga_ref_width; ++x)
                    {
                        const int i = y * vga_ref_width + x;

                        if (vid_mask_buffer_[i] &&
                            previous_ui[i] != vid_ui_buffer_[i])
                        {
                            ui_dirty = true;
                            break;
                        }
                    }
                }
            }
            else
            {
                ui_dirty = previous_ui != vid_ui_buffer_;
            }
        }

		if (ui_dirty)
		{
			auto* texels = static_cast<u32*>(
				gpu_ui_texture.data
			);

			for (int y = 0; y < vga_ref_height; ++y)
			{
				for (int x = 0; x < vga_ref_width; ++x)
				{
					const int src_offset =
						y * vga_ref_width + x;

					const u32 color = palette_[
						vid_ui_buffer_[src_offset]
					];

					// Respetar la máscara de transparencia
					// original de BStone.
					const bool visible =
						!vid_is_hud ||
						(
							y >= ref_3d_view_top_y &&
							y <= ref_3d_view_bottom_y &&
							vid_mask_buffer_[src_offset]
						);

					const u32 rgba =
						(color << 8) |
						(visible ? 0xFFU : 0x00U);

					texels[swizzle_y[y] + swizzle_x[x]] = rgba;

				}
			}

			C3D_TexFlush(&gpu_ui_texture);

            previous_ui = vid_ui_buffer_;
            previous_mask = vid_mask_buffer_;
            previous_palette = palette_;
            previous_hud = vid_is_hud;
            ui_cache_valid = true;

		}

		profile_marks[3] = ProfileClock::now();
		// ---------------------------------------------
		// 3. Definir las imágenes para Citro2D.
		// ---------------------------------------------

		Tex3DS_SubTexture screen_subtexture{};

		const int screen_top = vid_is_hud ?
			vga_3d_view_top_y : 0;

		const int screen_height = vid_is_hud ?
			viewheight : vga_height;

		screen_subtexture.width = vga_width;
		screen_subtexture.height = vga_height;
		screen_subtexture.left = 0.0f;
		screen_subtexture.right =
			float(vga_width) / texture_width;
		screen_subtexture.top = 1.0f;
		screen_subtexture.bottom =
			1.0f - float(vga_height) / texture_height;

		C2D_Image screen_image{
			&gpu_screen_texture,
			&screen_subtexture
		};

		C2D_Image screen_image_right{
			&gpu_screen_texture_right,
			&screen_subtexture
		};

		Tex3DS_SubTexture ui_subtexture{};

		const int ui_top = vid_is_hud ?
			ref_3d_view_top_y : 0;

		const int ui_height = vid_is_hud ?
			ref_3d_view_height : vga_ref_height;

		ui_subtexture.width = vga_ref_width;
		ui_subtexture.height = vga_ref_height;
		ui_subtexture.left = 0.0f;
		ui_subtexture.right =
			float(vga_ref_width) / texture_width;
		ui_subtexture.top = 1.0f;
		ui_subtexture.bottom =
			1.0f - float(vga_ref_height) / texture_height;

		C2D_Image ui_image{
			&gpu_ui_texture,
			&ui_subtexture
		};

		// ---------------------------------------------
		// 4. Dibujar cada perspectiva en su ojo.
		// ---------------------------------------------

	C3D_RenderTarget* top_targets[] =
		{
			gpu_top_,
			gpu_top_right_
		};

		for (int eye = 0; eye < 2; ++eye)
		{
			auto* target = top_targets[eye];

			C2D_TargetClear(
				target,
				C2D_Color32(0, 0, 0, 255));

			C2D_SceneBegin(target);

			if (vid_is_hud)
			{
				// Stereo eye order diagnostic.
				// Diagnostic: identical image for both eyes.
				const C2D_Image& eye_image =
					eye == 1 && stereo_active ?
					screen_image_right :
					screen_image;

				C2D_DrawImageAt(
					eye_image,
					0.0f,
					0.0f,
					0.5f,
					nullptr,
					400.0f / vga_width,
					240.0f / vga_height);
			}

			// The 2D user interface is identical for both eyes.
			C2D_DrawImageAt(
				ui_image,
				0.0f,
				0.0f,
				0.6f,
				nullptr,
				400.0f / vga_ref_width,
				240.0f / vga_ref_height);
		}

		// ---------------------------------------------
		// Bottom screen HUD.
		// ---------------------------------------------

		C2D_TargetClear(
			gpu_bottom_,
			C2D_Color32(12, 20, 30, 255));

		C2D_SceneBegin(gpu_bottom_);

        if (vid_is_hud)
        {
            n3ds_hud::draw();

            // Temporary performance overlay.
            C2D_DrawRectSolid(
                8, 65, 0.79f,
                190, 29,
                C2D_Color32(2, 9, 5, 255));

            n3ds_hud::draw_text(
                12, 68,
                profile_line1,
                C2D_Color32(168, 255, 179, 255));

            n3ds_hud::draw_text(
                12, 80,
                profile_line2,
                C2D_Color32(168, 255, 179, 255));
        }
        else
        {
            n3ds_intro::draw();
        }

        C3D_FrameEnd(0);

        profile_marks[4] = ProfileClock::now();

        if (vid_is_hud)
        {
            // Don't mix 2D and stereoscopic samples.
            if (profile_stereo != stereo_active)
            {
                profile_stereo = stereo_active;
                profile_samples = 0;
				profile_pixels_sum = 0.0;
				profile_flush_sum = 0.0;

                for (double& value : profile_sums)
                    value = 0.0;

                std::snprintf(
                    profile_line1,
                    sizeof(profile_line1),
                    "MEASURING %s...",
                    stereo_active ? "3D" : "2D");

                profile_line2[0] = '\0';
            }

            for (int i = 0; i < 4; ++i)
            {
                profile_sums[i] +=
                    std::chrono::duration<double, std::milli>(
                        profile_marks[i + 1] -
                        profile_marks[i]).count();
            }

			profile_pixels_sum += profile_pixels_frame;
			profile_flush_sum += profile_flush_frame;
            ++profile_samples;

            if (profile_samples >= 15)
            {
                const double count =
                    static_cast<double>(profile_samples);

				std::snprintf(
					profile_line1,
					sizeof(profile_line1),
					"%s W%.1f PIX%.1f FL%.1f",
					profile_stereo ? "3D" : "2D",
					profile_sums[0] / count,
					profile_pixels_sum / count,
					profile_flush_sum / count);

                std::snprintf(
                    profile_line2,
                    sizeof(profile_line2),
                    "UI %.1f DRAW %.1f",
                    profile_sums[2] / count,
                    profile_sums[3] / count);

				for (double& value : profile_sums)
					value = 0.0;

				profile_pixels_sum = 0.0;
				profile_flush_sum = 0.0;

				profile_samples = 0;

            }
        }

				// Yield CPU time for audio without
				// imposing a full millisecond per frame.
				svcSleepThread(250000LL);

		return;

} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::get_palette(int offset, int count, std::uint8_t* vga_palette) const
try {
	BSTONE_ASSERT(offset >= 0);
	BSTONE_ASSERT(count >= 0);
	BSTONE_ASSERT((offset + count) <= 256);
	BSTONE_ASSERT(vga_palette);

	auto& dst_vga_palette = *reinterpret_cast<VgaPalette*>(vga_palette);

	std::copy_n(
		vga_palette_.cbegin() + offset,
		count,
		dst_vga_palette.begin()
	);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::fill_palette(int r, int g, int b) noexcept
{
	for (auto& vga_color : vga_palette_)
	{
		vga_color[0] = static_cast<std::uint8_t>(r);
		vga_color[1] = static_cast<std::uint8_t>(g);
		vga_color[2] = static_cast<std::uint8_t>(b);
	}

	update_palette_from_vga(0, 256);
}

void SwVideo::set_palette(int offset, int count, const std::uint8_t* vga_palette)
try {
	BSTONE_ASSERT(offset >= 0);
	BSTONE_ASSERT(count >= 0);
	BSTONE_ASSERT((offset + count) <= 256);
	BSTONE_ASSERT(vga_palette);

	const auto& src_vga_palette = *reinterpret_cast<const VgaPalette*>(vga_palette);

	std::copy_n(
		src_vga_palette.cbegin(),
		count,
		vga_palette_.begin() + offset);

	update_palette_from_vga(offset, count);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::fade_out(int start, int end, int red, int green, int blue, int steps)
try {
	if (!gp_no_fade_in_or_out())
	{
		auto palette1 = VgaPalette{};
		auto palette2 = VgaPalette{};

		get_palette(0, 256, &palette1[0][0]);

		std::copy_n(&palette1[0][0], 768, &palette2[0][0]);

		//
		// fade through intermediate frames
		//
		for (int i = 0; i < steps; ++i)
		{
			auto origptr = &palette1[start][0];
			auto newptr = &palette2[start][0];

			for (int j = start; j <= end; ++j)
			{
				auto orig = *origptr++;
				auto delta = red - orig;
				*newptr++ = static_cast<std::uint8_t>(orig + ((delta * i) / steps));

				orig = *origptr++;
				delta = green - orig;
				*newptr++ = static_cast<std::uint8_t>(orig + ((delta * i) / steps));

				orig = *origptr++;
				delta = blue - orig;
				*newptr++ = static_cast<std::uint8_t>(orig + ((delta * i) / steps));
			}

			set_palette(0, 256, &palette2[0][0]);

			present();

			if (!vid_has_vsync)
			{
				VL_WaitVBL(1);
			}
		}
	}

	//
	// final color
	//
	fill_palette(red, green, blue);

	present();

	if (!vid_has_vsync)
	{
		VL_WaitVBL(1);
	}

	screenfaded = true;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::fade_in(int start, int end, const std::uint8_t* palette, int steps)
try {
	if (!gp_no_fade_in_or_out())
	{
		auto palette1 = VgaPalette{};
		auto palette2 = VgaPalette{};

		VL_GetPalette(0, 256, &palette1[0][0]);

		palette2 = palette1;

		const auto& src_palette = *reinterpret_cast<const VgaPalette*>(palette);

		//
		// fade through intermediate frames
		//
		for (auto i = 0; i < steps; ++i)
		{
			for (auto j = start; j <= end; ++j)
			{
				for (auto k = 0; k < 3; ++k)
				{
					const auto delta = src_palette[j][k] - palette1[j][k];

					palette2[j][k] = static_cast<std::uint8_t>(palette1[j][k] + ((delta * i) / steps));
				}
			}

			VL_SetPalette(0, 256, &palette2[0][0]);

			present();

			if (!vid_has_vsync)
			{
				VL_WaitVBL(1);
			}
		}
	}

	//
	// final color
	//
	VL_SetPalette(0, 256, palette);

	present();

	if (!vid_has_vsync)
	{
		VL_WaitVBL(1);
	}

	screenfaded = false;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::initialize_video()
try {
	vid_initialize_common();
	calculate_dimensions();

	vid_initialize_vanilla_raycaster();

	initialize_palette();
	initialize_vga_buffer();
	vid_initialize_ui_buffer();

	if (!C3D_Init(C3D_DEFAULT_CMDBUF_SIZE))
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to initialize Citro3D.");
	}

	citro3d_initialized_ = true;

	if (!C2D_Init(C2D_DEFAULT_MAX_OBJECTS))
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to initialize Citro2D.");
	}

	citro2d_initialized_ = true;
	C2D_Prepare();

	// Enable native 3DS stereoscopic display.
	gfxSet3D(true);

	// Left eye.
	gpu_top_ = C2D_CreateScreenTarget(
		GFX_TOP,
		GFX_LEFT);

	if (!gpu_top_)
	{
		BSTONE_THROW_STATIC_SOURCE(
			"Failed to create left eye render target.");
	}

	// Right eye.
	gpu_top_right_ = C2D_CreateScreenTarget(
		GFX_TOP,
		GFX_RIGHT);

	if (!gpu_top_right_)
	{
		BSTONE_THROW_STATIC_SOURCE(
			"Failed to create right eye render target.");
	}

	gfxSetScreenFormat(GFX_BOTTOM, GSP_BGR8_OES);

	gpu_bottom_ = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

	if (!gpu_bottom_)
	{
		BSTONE_THROW_STATIC_SOURCE(
			"Failed to create bottom GPU screen target.");
	}

} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::initialize_vga_buffer()
try {
	const auto area = 2 * vga_width * vga_height;

	sw_vga_buffer_ = VgaBuffer{};
	sw_vga_buffer_.resize(area);

	vga_memory = sw_vga_buffer_.data();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::initialize_palette()
{
	vga_palette_ = {};
	palette_ = {};
}

void SwVideo::calculate_dimensions()
{
    const auto parameters = vid_create_screen_size_param();
    vid_calculate_window_elements_dimensions(parameters, vid_layout_);
    vid_calculate_vga_dimensions();
}

void SwVideo::uninitialize_gpu() noexcept
{
    if (citro3d_initialized_)
    {
        C3D_FrameSync();
        if (gpu_textures_ready)
        {
            C3D_TexDelete(&gpu_ui_texture);
            C3D_TexDelete(&gpu_screen_texture_right);
            C3D_TexDelete(&gpu_screen_texture);
            gpu_textures_ready = false;
        }
        if (gpu_bottom_) C3D_RenderTargetDelete(gpu_bottom_);
        if (gpu_top_right_) C3D_RenderTargetDelete(gpu_top_right_);
        if (gpu_top_) C3D_RenderTargetDelete(gpu_top_);
        gpu_bottom_ = gpu_top_right_ = gpu_top_ = nullptr;
        if (citro2d_initialized_)
        {
            C2D_Fini();
            citro2d_initialized_ = false;
        }
        C3D_Fini();
        citro3d_initialized_ = false;
    }
}

void SwVideo::uninitialize_vga_buffer() noexcept
{
	sw_vga_buffer_.clear();
	vga_memory = nullptr;
}

void SwVideo::update_palette_from_vga(int offset, int count)
{
	for (auto i = 0; i < count; ++i)
	{
		const auto& vga_color = vga_palette_[offset + i];
		auto& color = palette_[offset + i];

		color =
			0xFF000000U |
			((((255U * vga_color[0]) / 63U) & 0xFFU) << 16) |
			((((255U * vga_color[1]) / 63U) & 0xFFU) << 8) |
			(((255U * vga_color[2]) / 63U) & 0xFFU);
	}
}

} // namespace

// ==========================================================================

VideoUPtr make_sw_video()
{
	return std::make_unique<SwVideo>();
}

} // bstone
