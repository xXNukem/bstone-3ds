/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// Software accelerated video (SW).

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
#include "bstone_video_cvars.h"
#include <atomic>
#include <array>
#include "bstone_r3r_utils.h"

#include "bstone_sys_video_mgr.h"
#include "bstone_sys_window_mgr.h"
#if defined(__3DS__) || defined(_3DS)
#include <3ds.h>
#include <citro2d.h>
#include "bstone_3ds_hud.h"
#include "bstone_3ds_intro.h"
#endif


#if defined(__3DS__) || defined(_3DS)
extern bool n3ds_stereo_active;
#endif
namespace bstone {

namespace {


#if defined(__3DS__) || defined(_3DS)

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

#endif


class SwVideo final : public Video
{
public:
	SwVideo(sys::VideoMgr& video_mgr, sys::WindowMgr& window_mgr);

	~SwVideo() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

	bool is_hardware() const noexcept override;
	StringView get_renderer_name() override;
	void clear_vga_buffer() override;
	void take_screenshot(
		int width,
		int height,
		int stride_rgb_888,
		ScreenshotBuffer&& src_pixels_rgb_888) override;
	void vsync_present() override;
	void present() override;

	void get_palette(int offset, int count, std::uint8_t* vga_palette) const override;
	void fill_palette(int r, int g, int b) noexcept override;
	void set_palette(int offset, int count, const std::uint8_t* vga_palette) override;

	void apply_widescreen() override;
	void apply_window_mode() override;
	void apply_filler_color_index() override;

	void fade_out(int start, int end, int red, int green, int blue, int steps) override;
	void fade_in(int start, int end, const std::uint8_t* palette, int steps) override;

	// HW
	//

	const Rgba8Palette& get_default_palette() const noexcept override;

	void enable_fizzle_fx(bool is_enabled) override;
	void enable_fizzle_fx_fading(bool is_fading) override;
	void set_fizzle_fx_color_index(int color_index) override;
	void set_fizzle_fx_ratio(float ratio) override;

	void clear_wall_render_list() noexcept override;
	void add_wall_render_item(int tile_x, int tile_y) override;

	void clear_pushwall_render_list() noexcept override;
	void add_pushwall_render_item(int tile_x, int tile_y) override;

	void clear_door_render_list() noexcept override;
	void add_door_render_item(int tile_x, int tile_y) override;

	void clear_static_render_list() noexcept override;
	void add_static_render_item(int bs_static_index) override;

	void clear_actor_render_list() noexcept override;
	void add_actor_render_item(int bs_actor_index) override;

	void on_load_level() override;
	void on_update_wall_switch(int x, int y) override;
	void on_move_pushwall() override;
	void on_step_pushwall(int old_x, int old_y) override;
	void on_pushwall_to_wall(int old_x, int old_y, int new_x, int new_y) override;
	void on_move_door(int door_index) override;
	void on_update_door_lock(int bs_door_index) override;
	void on_remove_static(const statobj_t& bs_static) override;
	void on_remove_actor(const objtype& bs_actor) override;

	void apply_vsync() override;
	void apply_msaa() override;
	void apply_texture_upscale() override;
	void apply_external_textures() override;
	void update_samplers() override;

	const R3rDeviceFeatures& get_device_features() const noexcept override;

	//
	// HW

private:
	static constexpr auto log_prefix = "[VIDSW] ";
	static const sys::Color opaque_black;

private:
	void initialize_video();
	void copy_texture_to_rendering_target(
		sys::Texture& texture,
		const sys::Rectangle* src_rect,
		const sys::Rectangle* dst_rect);
	void enable_texture_blending(sys::Texture& texture, bool is_enable);
	void fill_rects(Span<const sys::Rectangle> rects);
	void initialize_vga_buffer();
	void create_window();
	void initialize_renderer();
	void create_screen_texture();
	void create_ui_texture();
	void uninitialize_textures();
	void initialize_textures();
	void initialize_palette();
	void calculate_dimensions();
	void uninitialize_vga_buffer() noexcept;
	void update_palette_from_vga(int offset, int count);

private:
	sys::VideoMgr& video_mgr_;
	sys::WindowMgr& window_mgr_;
	sys::WindowUPtr window_{};
	sys::RendererUPtr renderer_{};
	std::string renderer_name_buffer_{};
	StringView renderer_name_{};
	sys::TextureUPtr screen_texture_{};
	sys::TextureUPtr ui_texture_{};
	VgaBuffer sw_vga_buffer_{};
	VgaPalette vga_palette_{};
	SdlPalette palette_{};
	sys::Rectangle ui_src_rect_{};
	sys::Rectangle ui_4x3_dst_rect_{};
	sys::Rectangle ui_wide_dst_rect_{};
	sys::Rectangle ui_top_src_rect_{};
	sys::Rectangle ui_4x3_top_dst_rect_{};
	sys::Rectangle ui_wide_top_dst_rect_{};
	sys::Rectangle ui_middle_src_rect_{};
	sys::Rectangle ui_4x3_middle_dst_rect_{};
	sys::Rectangle ui_wide_middle_dst_rect_{};
	sys::Rectangle ui_bottom_src_rect_{};
	sys::Rectangle ui_4x3_bottom_dst_rect_{};
	sys::Rectangle ui_wide_bottom_dst_rect_{};
	std::array<sys::Rectangle, 2> filler_ui_rects_{};
	std::array<sys::Rectangle, 4> filler_hud_rects_{};
	sys::Rectangle screen_dst_rect_{};
	sys::Color filler_color_{};

	#if defined(__3DS__) || defined(_3DS)
		C3D_RenderTarget* gpu_top_{};
		C3D_RenderTarget* gpu_top_right_{};
		C3D_RenderTarget* gpu_bottom_{};

		N3dsEyeWorker eye_worker_{};
	#endif



	// HW
	//

	Rgba8Palette default_palette_{};
	R3rDeviceFeatures device_features_{};

	//
	// HW
};

// ==========================================================================

using SwVideoPool = bstone::SinglePoolResource<SwVideo>;
SwVideoPool sw_video_pool{};

// ==========================================================================

constexpr sys::Color SwVideo::opaque_black = sys::Color{0, 0, 0, 0xFF};

SwVideo::SwVideo(sys::VideoMgr& video_mgr, sys::WindowMgr& window_mgr)
try
	:
	video_mgr_{video_mgr},
	window_mgr_{window_mgr}
{
	initialize_video();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED


SwVideo::~SwVideo()
{
#if defined(__3DS__) || defined(_3DS)
    eye_worker_.shutdown();
#endif

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

bool SwVideo::is_hardware() const noexcept
{
	return false;
}

StringView SwVideo::get_renderer_name()
{
	return renderer_name_;
}

void SwVideo::clear_vga_buffer()
{
	std::fill(sw_vga_buffer_.begin(), sw_vga_buffer_.end(), VgaBuffer::value_type{});
}

void SwVideo::apply_widescreen()
try {
	screen_texture_ = nullptr;

	uninitialize_vga_buffer();
	calculate_dimensions();
	initialize_vga_buffer();
	create_screen_texture();
	renderer_->set_viewport();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::take_screenshot(
	int width,
	int height,
	int stride_rgb_888,
	ScreenshotBuffer&& src_pixels_rgb_888)
try {
	renderer_->read_pixels(sys::PixelFormat::r8g8b8, src_pixels_rgb_888.get(), stride_rgb_888);

	vid_schedule_save_screenshot_task(
		width,
		height,
		stride_rgb_888,
		std::move(src_pixels_rgb_888),
		false);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::vsync_present()
try {
	// Clear all
	//
	renderer_->set_draw_color(opaque_black);
	renderer_->clear();

	// Present
	//
	renderer_->present();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::present()
try {
	#if defined(__3DS__) || defined(_3DS)
		const auto start = std::chrono::steady_clock::now();
	#endif
	
	#if defined(__3DS__) || defined(_3DS)

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


		static C3D_Tex gpu_screen_texture{};
		static C3D_Tex gpu_screen_texture_right{};
		static C3D_Tex gpu_ui_texture{};

		static bool gpu_textures_ready = false;
		static bool gpu_screen_format_ready = false;

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
            static SdlPalette palette565_source{};
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
        static SdlPalette previous_palette{};
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

	#endif

	// HUD+3D stuff
	//
	if (vid_is_hud)
	{
		const auto texture_lock = screen_texture_->make_lock();

		const auto& src_pixels = sw_vga_buffer_;
		const auto src_pitch = vga_width;

		const auto dst_pixels = texture_lock->get_pixels<std::uint32_t*>();
		const auto dst_bytes_pitch = texture_lock->get_pitch();
		const auto dst_pitch = dst_bytes_pitch / 4;

		auto src_line_offset = 0;
		auto dst_line_offset = 0;

		for (auto y = decltype(vga_height){}; y < vga_height; ++y)
		{
			const auto src_line = &src_pixels[src_line_offset];
			auto dst_line = &dst_pixels[dst_line_offset];

			for (auto x = decltype(vga_width){}; x < vga_width; ++x)
			{
				dst_line[x] = palette_[src_line[x]];
			}

			src_line_offset += src_pitch;
			dst_line_offset += dst_pitch;
		}
	}

	// 2D stuff
	//
	{
		const auto texture_lock = ui_texture_->make_lock();

		const auto dst_pixels = texture_lock->get_pixels<std::uint32_t*>();
		const auto dst_bytes_pitch = texture_lock->get_pitch();
		const auto dst_pitch = dst_bytes_pitch / 4;

		auto src_line_offset = 0;
		auto dst_line_offset = 0;

		const auto alpha_0_mask = 0x00FFFFFFU;

		for (auto y = decltype(vga_ref_height){}; y < vga_ref_height; ++y)
		{
			auto dst_line = &dst_pixels[dst_line_offset];

			for (auto x = decltype(vga_ref_width){}; x < vga_ref_width; ++x)
			{
				const auto src_offset = src_line_offset + x;
				auto dst_color = palette_[vid_ui_buffer_[src_offset]];

				if (vid_is_hud)
				{
					if (!vid_mask_buffer_[src_offset])
					{
						dst_color &= alpha_0_mask;
					}
				}

				dst_line[x] = dst_color;
			}

			src_line_offset += vga_ref_width;
			dst_line_offset += dst_pitch;
		}
	}

	// Clear all
	//
	renderer_->set_draw_color(opaque_black);
	renderer_->clear();

	// Copy HUD+3D stuff
	//
	if (vid_is_hud)
	{
		copy_texture_to_rendering_target(*screen_texture_, nullptr, &screen_dst_rect_);
	}

	// Copy 2D stuff
	//
	if (vid_is_hud)
	{
		enable_texture_blending(*ui_texture_, true);
	}

	const auto is_stretched = vid_cfg_is_ui_stretched();
	const auto is_widescreen = vid_cfg_is_widescreen();

	const auto is_top_wide = is_stretched;
	const auto is_middle_wide = (vid_is_hud && is_widescreen) || (!vid_is_hud && is_stretched);
	const auto is_bottom_wide = is_stretched;

	if (false)
	{}
	else if (is_top_wide && is_middle_wide && is_bottom_wide)
	{
		copy_texture_to_rendering_target(*ui_texture_, nullptr, &ui_wide_dst_rect_);
	}
	else if (!is_top_wide && !is_middle_wide && !is_bottom_wide)
	{
		copy_texture_to_rendering_target(*ui_texture_, nullptr, &ui_4x3_dst_rect_);
	}
	else
	{
		const auto& dst_top_rect = is_top_wide ? ui_wide_top_dst_rect_ : ui_4x3_top_dst_rect_;
		const auto& dst_middle_rect = is_middle_wide ? ui_wide_middle_dst_rect_ : ui_4x3_middle_dst_rect_;
		const auto& dst_bottom_rect = is_bottom_wide ? ui_wide_bottom_dst_rect_ : ui_4x3_bottom_dst_rect_;

		copy_texture_to_rendering_target(*ui_texture_, &ui_top_src_rect_, &dst_top_rect);
		copy_texture_to_rendering_target(*ui_texture_, &ui_middle_src_rect_, &dst_middle_rect);
		copy_texture_to_rendering_target(*ui_texture_, &ui_bottom_src_rect_, &dst_bottom_rect);
	}

	if (vid_is_hud)
	{
		enable_texture_blending(*ui_texture_, false);
	}

	// Use filler if necessary
	//
	if (!vid_cfg_is_ui_stretched())
	{
		const auto is_hud = vid_is_hud;

		auto fill_color = opaque_black;

		if (!vid_is_movie)
		{
			fill_color = filler_color_;
		}

		renderer_->set_draw_color(fill_color);

		if (is_hud)
		{
			fill_rects(make_const_span(filler_hud_rects_.data(), static_cast<std::intptr_t>(filler_hud_rects_.size())));
		}
		else
		{
			fill_rects(make_const_span(filler_ui_rects_.data(), static_cast<std::intptr_t>(filler_ui_rects_.size())));
		}
	}

	if (vid_is_take_screenshot_scheduled)
	{
		vid_take_screenshot();
	}

	// Present
	//

	#if defined(__3DS__) || defined(_3DS)
		const auto before_present = std::chrono::steady_clock::now();
	#endif

	renderer_->present();

	#if defined(__3DS__) || defined(_3DS)

		const auto end = std::chrono::steady_clock::now();

		static double preparation_ms = 0.0;
		static double presentation_ms = 0.0;
		static int frames = 0;

		preparation_ms += std::chrono::duration<double, std::milli>(
			before_present - start).count();

		presentation_ms += std::chrono::duration<double, std::milli>(
			end - before_present).count();

		frames++;

		if (frames >= 20)
		{
			FILE* file = std::fopen(
				"sdmc:/3ds/bstone/video_performance.txt", "a");

			if (file)
			{
				std::fprintf(file,
					"Preparation: %.2f ms | SDL Present: %.2f ms\n",
					preparation_ms / frames,
					presentation_ms / frames);

				std::fclose(file);
			}

			preparation_ms = 0.0;
			presentation_ms = 0.0;
			frames = 0;
		}

	#endif
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

void SwVideo::apply_window_mode()
try {
	sys::Window& window = *window_;

	auto param = R3rUtilsSetWindowModeParam{};
	param.display_mode.width = vid_cfg_get_width();
	param.display_mode.height = vid_cfg_get_height();
	param.display_mode.refresh_rate = vid_cfg_get_refresh_rate();
	param.fullscreen_mode = R3rUtils::get_fullscreen_mode_from_cvar();
	R3rUtils::set_window_mode(window, param);

	R3rUtils::set_fullscreen_mode_cvar_from_window(window);

	calculate_dimensions();
	vid_initialize_vanilla_raycaster();
	vid_initialize_common();
	uninitialize_textures();
	initialize_textures();
	initialize_vga_buffer();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::apply_filler_color_index()
try {
	filler_color_.r = static_cast<std::uint8_t>((255 * vgapal[(vid_cfg_get_filler_color_index() * 3) + 0]) / 63);
	filler_color_.g = static_cast<std::uint8_t>((255 * vgapal[(vid_cfg_get_filler_color_index() * 3) + 1]) / 63);
	filler_color_.b = static_cast<std::uint8_t>((255 * vgapal[(vid_cfg_get_filler_color_index() * 3) + 2]) / 63);
	filler_color_.a = 0xFF;
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

// HW
//

const Rgba8Palette& SwVideo::get_default_palette() const noexcept
{
	return default_palette_;
}

void SwVideo::enable_fizzle_fx(bool) {}

void SwVideo::enable_fizzle_fx_fading(bool) {}

void SwVideo::set_fizzle_fx_color_index(int) {}

void SwVideo::set_fizzle_fx_ratio(float) {}

void SwVideo::clear_wall_render_list() noexcept {}

void SwVideo::add_wall_render_item(int, int) {}

void SwVideo::clear_pushwall_render_list() noexcept {}

void SwVideo::add_pushwall_render_item(int, int) {}

void SwVideo::clear_door_render_list() noexcept {}

void SwVideo::add_door_render_item(int, int) {}

void SwVideo::clear_static_render_list() noexcept {}

void SwVideo::add_static_render_item(int) {}

void SwVideo::clear_actor_render_list() noexcept {}

void SwVideo::add_actor_render_item(int) {}

void SwVideo::on_load_level() {}

void SwVideo::on_update_wall_switch(int, int) {}

void SwVideo::on_move_pushwall() {}

void SwVideo::on_step_pushwall(int, int) {}

void SwVideo::on_pushwall_to_wall(int, int, int, int) {}

void SwVideo::on_move_door(int) {}

void SwVideo::on_update_door_lock(int) {}

void SwVideo::on_remove_static(const statobj_t&) {}

void SwVideo::on_remove_actor(const objtype&) {}

void SwVideo::apply_vsync() {}

void SwVideo::apply_msaa() {}

void SwVideo::apply_texture_upscale() {}

void SwVideo::apply_external_textures() {}

void SwVideo::update_samplers() {}

const R3rDeviceFeatures& SwVideo::get_device_features() const noexcept
{
	return device_features_;
}

//
// HW

void SwVideo::initialize_video()
try {
	vid_initialize_common();
	calculate_dimensions();

	vid_initialize_vanilla_raycaster();

	create_window();
	initialize_renderer();
	initialize_textures();
	initialize_palette();
	initialize_vga_buffer();
	vid_initialize_ui_buffer();

	renderer_name_buffer_.reserve(128);
	renderer_name_buffer_ += "sw (";
	const auto sys_renderer_name = get_renderer_name();

	renderer_name_buffer_.append(
		sys_renderer_name.get_data(),
		static_cast<std::size_t>(sys_renderer_name.get_size()));

	renderer_name_buffer_ += ')';

	renderer_name_ = StringView{
		renderer_name_buffer_.data(),
		static_cast<std::intptr_t>(renderer_name_buffer_.size())};

	const auto window_title = vid_get_window_title_for_renderer(renderer_name_);
	window_->set_title(window_title.c_str());
	window_->show(true);

	#if defined(__3DS__) || defined(_3DS)

		// SDL2 ya ha inicializado el sistema gráfico.
		// Inicializamos Citro3D sin llamar a gfxInitDefault().
    
		if (!C3D_Init(C3D_DEFAULT_CMDBUF_SIZE))
		{
			BSTONE_THROW_STATIC_SOURCE("Failed to initialize Citro3D.");
		}

		if (!C2D_Init(C2D_DEFAULT_MAX_OBJECTS))
		{
			C3D_Fini();
			BSTONE_THROW_STATIC_SOURCE("Failed to initialize Citro2D.");
		}

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

	#endif

} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::copy_texture_to_rendering_target(
	sys::Texture& texture,
	const sys::Rectangle* src_rect,
	const sys::Rectangle* dst_rect)
try {
	texture.copy(src_rect, dst_rect);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::enable_texture_blending(sys::Texture& texture, bool is_enable)
try {
	texture.set_blend_mode(is_enable ? sys::TextureBlendMode::blend : sys::TextureBlendMode::none);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::fill_rects(Span<const sys::Rectangle> rects)
try {
	renderer_->fill(rects);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::initialize_vga_buffer()
try {
	const auto area = 2 * vga_width * vga_height;

	sw_vga_buffer_ = VgaBuffer{};
	sw_vga_buffer_.resize(area);

	vga_memory = sw_vga_buffer_.data();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::create_window()
try {
	const auto is_native_mode = vid_is_native_mode();
	const auto title = vid_get_game_name_and_game_version_string();

	auto param = sys::WindowInitParam{};
	param.title = title.c_str();
	param.x = sys::WindowOffset::make_centered();
	param.y = sys::WindowOffset::make_centered();

	if (!is_native_mode && vid_cfg_is_positioned())
	{
		param.x = sys::WindowOffset{vid_cfg_get_x()};
		param.y = sys::WindowOffset{vid_cfg_get_y()};
	}

	param.width = vid_cfg_get_width();
	param.height = vid_cfg_get_height();

	param.is_visible = true;
	param.fullscreen_type = is_native_mode ? sys::WindowFullscreenType::fake : sys::WindowFullscreenType::none;
	window_ = window_mgr_.make_window(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::initialize_renderer()
try {
	auto renderer_param = sys::RendererInitParam{};
	renderer_param.is_vsync = vid_cfg_is_vsync();

	renderer_ = window_->make_renderer(renderer_param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED


void SwVideo::create_screen_texture()
try {
#if defined(__3DS__) || defined(_3DS)
    // The 3DS backend uses Citro3D textures.
    // No SDL screen texture is required.
    return;
#else
    auto param = sys::TextureInitParam{};
    param.pixel_format = sys::PixelFormat::b8g8r8a8;
    param.access = sys::TextureAccess::streaming;
    param.width = vga_width;
    param.height = vga_height;

    screen_texture_ = renderer_->make_texture(param);
#endif
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED



void SwVideo::create_ui_texture()
try {
#if defined(__3DS__) || defined(_3DS)
    // Citro3D handles the 3DS UI texture.
    return;
#else
    auto param = sys::TextureInitParam{};
    param.pixel_format = sys::PixelFormat::b8g8r8a8;
    param.access = sys::TextureAccess::streaming;
    param.width = vga_ref_width;
    param.height = vga_ref_height;

    ui_texture_ = renderer_->make_texture(param);
#endif
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED


void SwVideo::uninitialize_textures()
{
	ui_texture_ = nullptr;
	screen_texture_ = nullptr;
}

void SwVideo::initialize_textures()
try {
	create_screen_texture();
	create_ui_texture();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::initialize_palette()
{
	vga_palette_ = {};
	palette_ = {};
}

void SwVideo::calculate_dimensions()
{
	auto src_param = vid_create_screen_size_param();
	vid_calculate_window_elements_dimensions(src_param, vid_layout_);
	vid_calculate_vga_dimensions();


	// UI whole rect
	//
	ui_src_rect_ = sys::Rectangle
	{
		0,
		0,
		vga_ref_width,
		vga_ref_height,
	};

	ui_4x3_dst_rect_ = sys::Rectangle
	{
		vid_layout_.window_viewport_left_width + vid_layout_.screen_left_filler_width,
		vid_layout_.window_viewport_top_height,
		vid_layout_.screen_width_4x3,
		vid_layout_.screen_height,
	};

	// UI stretched rect
	//
	ui_wide_dst_rect_ = sys::Rectangle
	{
		vid_layout_.window_viewport_left_width,
		vid_layout_.window_viewport_top_height,
		vid_layout_.screen_width,
		vid_layout_.screen_height,
	};

	// UI top rect
	//
	ui_top_src_rect_ = sys::Rectangle
	{
		0,
		0,
		vga_ref_width,
		ref_top_bar_height,
	};

	ui_4x3_top_dst_rect_ = sys::Rectangle
	{
		vid_layout_.window_viewport_left_width + vid_layout_.screen_left_filler_width,
		vid_layout_.window_viewport_top_height,
		vid_layout_.screen_width_4x3,
		vid_layout_.screen_top_filler_height,
	};

	ui_wide_top_dst_rect_ = sys::Rectangle
	{
		vid_layout_.window_viewport_left_width,
		vid_layout_.window_viewport_top_height,
		vid_layout_.screen_width,
		vid_layout_.screen_top_filler_height,
	};

	// UI middle rect (stretched to full width)
	//
	ui_middle_src_rect_ = sys::Rectangle
	{
		0,
		ref_view_top_y,
		vga_ref_width,
		ref_view_height,
	};

	ui_4x3_middle_dst_rect_ = sys::Rectangle
	{
		vid_layout_.window_viewport_left_width + vid_layout_.screen_left_filler_width,
		vid_layout_.window_viewport_top_height + vid_layout_.screen_top_filler_height,
		vid_layout_.screen_width_4x3,
		vid_layout_.screen_height - vid_layout_.screen_top_filler_height - vid_layout_.screen_bottom_filler_height,
	};

	ui_wide_middle_dst_rect_ = sys::Rectangle
	{
		vid_layout_.window_viewport_left_width,
		vid_layout_.window_viewport_top_height + vid_layout_.screen_top_filler_height,
		vid_layout_.screen_width,
		vid_layout_.screen_height - vid_layout_.screen_top_filler_height - vid_layout_.screen_bottom_filler_height,
	};

	// UI bottom rect
	//
	ui_bottom_src_rect_ = sys::Rectangle
	{
		0,
		ref_view_bottom_y + 1,
		vga_ref_width,
		ref_bottom_bar_height,
	};

	ui_4x3_bottom_dst_rect_ = sys::Rectangle
	{
		vid_layout_.window_viewport_left_width + vid_layout_.screen_left_filler_width,
		vid_layout_.window_viewport_top_height + vid_layout_.screen_height - vid_layout_.screen_bottom_filler_height,
		vid_layout_.screen_width_4x3,
		vid_layout_.screen_bottom_filler_height,
	};

	ui_wide_bottom_dst_rect_ = sys::Rectangle
	{
		vid_layout_.window_viewport_left_width,
		vid_layout_.window_viewport_top_height + vid_layout_.screen_height - vid_layout_.screen_bottom_filler_height,
		vid_layout_.screen_width,
		vid_layout_.screen_bottom_filler_height,
	};

	// UI left bar
	filler_ui_rects_[0] = sys::Rectangle
	{
		vid_layout_.window_viewport_left_width,
		vid_layout_.window_viewport_top_height,
		vid_layout_.screen_left_filler_width,
		vid_layout_.screen_height,
	};

	// UI right bar
	filler_ui_rects_[1] = sys::Rectangle
	{
		vid_layout_.window_viewport_left_width + vid_layout_.screen_width - vid_layout_.screen_left_filler_width,
		vid_layout_.window_viewport_top_height,
		vid_layout_.screen_left_filler_width,
		vid_layout_.screen_height,
	};

	// HUD upper left rect
	filler_hud_rects_[0] = sys::Rectangle
	{
		vid_layout_.window_viewport_left_width,
		vid_layout_.window_viewport_top_height,
		vid_layout_.screen_left_filler_width,
		vid_layout_.screen_top_filler_height,
	};

	// HUD upper right rect
	filler_hud_rects_[1] = sys::Rectangle
	{
		vid_layout_.window_viewport_left_width + vid_layout_.screen_width - vid_layout_.screen_right_filler_width,
		vid_layout_.window_viewport_top_height,
		vid_layout_.screen_right_filler_width,
		vid_layout_.screen_top_filler_height,
	};

	// HUD lower left rect
	filler_hud_rects_[2] = sys::Rectangle
	{
		vid_layout_.window_viewport_left_width,
		vid_layout_.window_viewport_top_height + vid_layout_.screen_height - vid_layout_.screen_bottom_filler_height,
		vid_layout_.screen_left_filler_width,
		vid_layout_.screen_bottom_filler_height,
	};

	// HUD lower right rect
	filler_hud_rects_[3] = sys::Rectangle
	{
		vid_layout_.window_viewport_left_width + vid_layout_.screen_width - vid_layout_.screen_right_filler_width,
		vid_layout_.window_viewport_top_height + vid_layout_.screen_height - vid_layout_.screen_bottom_filler_height,
		vid_layout_.screen_right_filler_width,
		vid_layout_.screen_bottom_filler_height,
	};

	apply_filler_color_index();

	// Screen destination rect.

	const auto screen_left = (
		vid_cfg_is_widescreen() ?
		0 :
		vid_layout_.window_viewport_left_width + vid_layout_.screen_left_filler_width);

	const auto screen_top = vid_layout_.window_viewport_top_height;
	const auto screen_width = (vid_cfg_is_widescreen() ? vid_layout_.screen_width : vid_layout_.screen_width_4x3);
	const auto screen_height = vid_layout_.screen_height;

	screen_dst_rect_ = sys::Rectangle
	{
		screen_left,
		screen_top,
		screen_width,
		screen_height,
	};
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

VideoUPtr make_sw_video(sys::VideoMgr& video_mgr, sys::WindowMgr& window_mgr)
{
	return std::make_unique<SwVideo>(video_mgr, window_mgr);
}

} // bstone
