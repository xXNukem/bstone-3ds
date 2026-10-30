project(bstone VERSION 1.3.4 LANGUAGES CXX)

#
# Version.
#
set(BSTONE_VERSION "${PROJECT_VERSION}")
set(BSTONE_VERSION_PRE_RELEASE "")

if (NOT ("${BSTONE_VERSION_PRE_RELEASE}" STREQUAL ""))
	set(BSTONE_VERSION "${BSTONE_VERSION}-${BSTONE_VERSION_PRE_RELEASE}")
endif ()

set(BSTONE_VERSION_SHORT "${BSTONE_VERSION}")

if (NOT ("${BSTONE_GIT_HASH}" STREQUAL ""))
	set(BSTONE_VERSION "${BSTONE_VERSION}+${BSTONE_GIT_HASH}")
endif ()

if (NOT ("${BSTONE_GIT_HASH_SHORT}" STREQUAL ""))
	set(BSTONE_VERSION_SHORT "${BSTONE_VERSION_SHORT}+${BSTONE_GIT_HASH_SHORT}")
endif ()

message(STATUS "[bstone] v${BSTONE_VERSION}")

# SDL2 must be a Nintendo 3DS installation. Use SDL2_DIR or
# CMAKE_PREFIX_PATH for an installation outside devkitPro's portlibs.
find_package(SDL2 2.0.4 CONFIG REQUIRED)
if (NOT TARGET SDL2::SDL2-static)
    message(FATAL_ERROR "A static Nintendo 3DS SDL2 installation is required (SDL2::SDL2-static).")
endif ()

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# Search only within the cross-compilation roots supplied by devkitPro.
find_path(BSTONE_CTRU_INCLUDE_DIR 3ds.h REQUIRED ONLY_CMAKE_FIND_ROOT_PATH)
find_path(BSTONE_CITRO2D_INCLUDE_DIR citro2d.h REQUIRED ONLY_CMAKE_FIND_ROOT_PATH)
find_path(BSTONE_CITRO3D_INCLUDE_DIR citro3d.h REQUIRED ONLY_CMAKE_FIND_ROOT_PATH)
find_library(BSTONE_CTRU_LIBRARY ctru REQUIRED ONLY_CMAKE_FIND_ROOT_PATH)
find_library(BSTONE_CITRO2D_LIBRARY citro2d REQUIRED ONLY_CMAKE_FIND_ROOT_PATH)
find_library(BSTONE_CITRO3D_LIBRARY citro3d REQUIRED ONLY_CMAKE_FIND_ROOT_PATH)

add_executable(bstone)

configure_file(
    "${PROJECT_SOURCE_DIR}/src/bstone_version_.h.in"
    "${PROJECT_BINARY_DIR}/src/bstone_version_.h"
)

set_target_properties(bstone PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_compile_definitions(bstone PRIVATE
    $<$<BOOL:${BSTONE_NO_ASSERT}>:BSTONE_NO_ASSERT>
)

# A missing return in a non-void function can compile to an ARM UDF trap.
target_compile_options(bstone PRIVATE -Werror=return-type)

if (BSTONE_MORE_COMPILER_WARNINGS)
    target_compile_options(bstone PRIVATE -Wall -Wextra -pedantic)
endif ()

target_link_libraries(bstone PRIVATE
    Threads::Threads
    bstone::lib::nuked_opl3
    bstone::lib::stb
    SDL2::SDL2-static
    ${BSTONE_CITRO2D_LIBRARY}
    ${BSTONE_CITRO3D_LIBRARY}
    ${BSTONE_CTRU_LIBRARY}
    m
)

target_include_directories(bstone PRIVATE
    src
    src/dosbox
    "${PROJECT_BINARY_DIR}/src"
    "${BSTONE_CTRU_INCLUDE_DIR}"
    "${BSTONE_CITRO2D_INCLUDE_DIR}"
    "${BSTONE_CITRO3D_INCLUDE_DIR}"
)

if (BSTONE_USE_PCH)
    target_precompile_headers(bstone PRIVATE src/bstone_precompiled.h)
endif ()

# ===========================================================================

set(BSTONE_LEGACY_SOURCES
	src/3d_act1.cpp
	src/3d_act2.cpp
	src/3d_agent.cpp
	src/3d_debug.cpp
	src/3d_def.h
	src/3d_draw.cpp
	src/3d_draw2.cpp
	src/3d_game.cpp
	src/3d_inter.cpp
	src/3d_main.cpp
	src/3d_menu.cpp
	src/3d_menu.h
	src/3d_msgs.cpp
	src/3d_play.cpp
	src/3d_scale.cpp
	src/3d_state.cpp
	src/an_codes.h
	src/audio.h
	src/colormap.cpp
	src/d3_d2.cpp
	src/d3_d2.h
	src/d3_dr2.cpp
	src/gfxv.h
	src/id_ca.cpp
	src/id_ca.h
	src/id_heads.h
	src/id_in.cpp
	src/id_in.h
	src/id_sd.cpp
	src/id_sd.h
	src/id_us.cpp
	src/id_us.h
	src/id_us_1.cpp
	src/id_vh.cpp
	src/id_vh.h
	src/id_vl.cpp
	src/id_vl.h
	src/jm_cio.h
	src/jm_free.cpp
	src/jm_lzh.cpp
	src/jm_lzh.h
	src/jm_tp.cpp
	src/jm_tp.h
	src/jm_vl.h
	src/markhack.cpp
	src/movie.cpp
	src/movie.h
	src/stub.cpp
	src/vgapal.cpp
)

target_sources(bstone PRIVATE ${BSTONE_LEGACY_SOURCES})
source_group("legacy" FILES ${BSTONE_LEGACY_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_LIB_DOSBOX_SOURCES
	src/dosbox/adlib.h
	src/dosbox/dbopl.cpp
	src/dosbox/dbopl.h
	src/dosbox/dosbox.h
	src/dosbox/hardware.h
	src/dosbox/inout.h
	src/dosbox/mixer.cpp
	src/dosbox/mixer.h
	src/dosbox/pic.h
	src/dosbox/setup.h
)

target_sources(bstone PRIVATE ${BSTONE_LIB_DOSBOX_SOURCES})
source_group("lib/dosbox" FILES ${BSTONE_LIB_DOSBOX_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_LIB_MISC_SOURCES
	src/bstone_algorithm.h
	src/bstone_api_string.h
	src/bstone_array.h
	src/bstone_ascii.h
	src/bstone_assert.h
	src/bstone_auto_arena_resource.cpp
	src/bstone_auto_arena_resource.h
	src/bstone_char_conv.cpp
	src/bstone_char_conv.h
	src/bstone_char_traits.h
	src/bstone_configurations.h
	src/bstone_crc32.cpp
	src/bstone_crc32.h
	src/bstone_cxx.h
	src/bstone_endian.h
	src/bstone_enum_flags.h
	src/bstone_exception.cpp
	src/bstone_exception.h
	src/bstone_file.cpp
	src/bstone_file.h
	src/bstone_file_posix.cpp
	src/bstone_file_stream.cpp
	src/bstone_file_stream.h
	src/bstone_four_cc.h
	src/bstone_fs.h
	src/bstone_fs_posix.cpp
	src/bstone_memory.h
	src/bstone_memory_binary_reader.cpp
	src/bstone_memory_binary_reader.h
	src/bstone_memory_resource.cpp
	src/bstone_memory_resource.h
	src/bstone_memory_stream.cpp
	src/bstone_memory_stream.h
	src/bstone_posix_string.cpp
	src/bstone_posix_string.h
	src/bstone_scope_exit.h
	src/bstone_sha1.cpp
	src/bstone_sha1.h
	src/bstone_single_pool_resource.h
	src/bstone_source_location.cpp
	src/bstone_source_location.h
	src/bstone_span.h
	src/bstone_static_ro_memory_stream.cpp
	src/bstone_static_ro_memory_stream.h
	src/bstone_stream.cpp
	src/bstone_stream.h
	src/bstone_string_view.h
	src/bstone_ts_auto_arena_resource.cpp
	src/bstone_ts_auto_arena_resource.h
	src/bstone_utility.h
)

target_sources(bstone PRIVATE ${BSTONE_LIB_MISC_SOURCES})
source_group("lib/misc" FILES ${BSTONE_LIB_MISC_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_SYS_SOURCES
	src/bstone_sys_audio_mgr.cpp
	src/bstone_sys_audio_mgr.h
	src/bstone_sys_audio_mgr_null.cpp
	src/bstone_sys_audio_mgr_null.h
	src/bstone_sys_event.h
	src/bstone_sys_event_mgr.cpp
	src/bstone_sys_event_mgr.h
	src/bstone_sys_event_mgr_null.cpp
	src/bstone_sys_event_mgr_null.h
	src/bstone_sys_keyboard_key.h
	src/bstone_sys_logger.cpp
	src/bstone_sys_logger.h
	src/bstone_sys_message_box.h
	src/bstone_sys_mouse.h
	src/bstone_sys_polling_audio_device.cpp
	src/bstone_sys_polling_audio_device.h
	src/bstone_sys_system_mgr.cpp
	src/bstone_sys_system_mgr.h
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_SYS_SOURCES})
source_group("bstone/sys" FILES ${BSTONE_BSTONE_SYS_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_SYS_SDL2_SOURCES
	src/bstone_sys_audio_mgr_sdl2.cpp
	src/bstone_sys_audio_mgr_sdl2.h
	src/bstone_sys_event_mgr_sdl2.cpp
	src/bstone_sys_event_mgr_sdl2.h
	src/bstone_sys_exception_sdl2.cpp
	src/bstone_sys_exception_sdl2.h
	src/bstone_sys_message_box_sdl2.cpp
	src/bstone_sys_polling_audio_device_sdl2.cpp
	src/bstone_sys_polling_audio_device_sdl2.h
	src/bstone_sys_sdl2_subsystem.cpp
	src/bstone_sys_sdl2_subsystem.h
	src/bstone_sys_system_mgr_sdl2.cpp
)


target_sources(bstone PRIVATE ${BSTONE_BSTONE_SYS_SDL2_SOURCES})
source_group("bstone/sys/sdl2" FILES ${BSTONE_BSTONE_SYS_SDL2_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_AUDIO_SOURCES
	src/bstone_audio_content_mgr.cpp
	src/bstone_audio_content_mgr.h
	src/bstone_audio_decoder.cpp
	src/bstone_audio_decoder.h
	src/bstone_audio_mixer.cpp
	src/bstone_audio_mixer.h
	src/bstone_audio_mixer_output_gains.h
	src/bstone_audio_mixer_utils.cpp
	src/bstone_audio_mixer_utils.h
	src/bstone_audio_mixer_validator.cpp
	src/bstone_audio_mixer_validator.h
	src/bstone_audio_mixer_voice_handle.cpp
	src/bstone_audio_mixer_voice_handle.h
	src/bstone_audio_mixer_voice_handle_mgr.h
	src/bstone_audio_sample_converter.cpp
	src/bstone_audio_sample_converter.h
	src/bstone_pc_speaker_audio_decoder.cpp
	src/bstone_pc_speaker_audio_decoder.h
	src/bstone_pcm_audio_decoder.cpp
	src/bstone_pcm_audio_decoder.h
	src/bstone_system_audio_mixer.cpp
	src/bstone_system_audio_mixer.h
	src/bstone_voice.h
	src/bstone_voice_group.cpp
	src/bstone_voice_group.h
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_AUDIO_SOURCES})
source_group("bstone/audio" FILES ${BSTONE_BSTONE_AUDIO_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_AUDIO_SOURCES
	src/bstone_adlib_decoder.cpp
	src/bstone_adlib_decoder.h
	src/bstone_adlib_music_decoder.cpp
	src/bstone_adlib_music_decoder.h
	src/bstone_adlib_sfx_decoder.cpp
	src/bstone_adlib_sfx_decoder.h
	src/bstone_dosbox_dbopl.cpp
	src/bstone_dosbox_dbopl.h
	src/bstone_nuked_opl3.cpp
	src/bstone_nuked_opl3.h
	src/bstone_opl3.cpp
	src/bstone_opl3.h
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_AUDIO_SOURCES})
source_group("bstone/audio/opl" FILES ${BSTONE_BSTONE_AUDIO_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_VIDEO_SOURCES
	src/bstone_fizzle_fx.cpp
	src/bstone_fizzle_fx.h
	src/bstone_generic_fizzle_fx.cpp
	src/bstone_generic_fizzle_fx.h
	src/bstone_ps_fizzle_fx.cpp
	src/bstone_ps_fizzle_fx.h
	src/bstone_sw_video.cpp
	src/bstone_sw_video.h
	src/bstone_video.cpp
	src/bstone_video.h
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_VIDEO_SOURCES})
source_group("bstone/video" FILES ${BSTONE_BSTONE_VIDEO_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_IMAGE_SOURCES
	src/bstone_image_encoder.cpp
	src/bstone_image_encoder.h
	src/bstone_sprite.cpp
	src/bstone_sprite.h
	src/bstone_sprite_cache.cpp
	src/bstone_sprite_cache.h
	src/bstone_stb_image_encoder.cpp
	src/bstone_stb_image_encoder.h
	src/bstone_stb_image_utils.cpp
	src/bstone_stb_image_utils.h
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_IMAGE_SOURCES})
source_group("bstone/image" FILES ${BSTONE_BSTONE_IMAGE_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_CVAR_SOURCES
	src/bstone_ccmd.cpp
	src/bstone_ccmd.h
	src/bstone_ccmd_action.cpp
	src/bstone_ccmd_action.h
	src/bstone_ccmd_mgr.cpp
	src/bstone_ccmd_mgr.h
	src/bstone_cvalidator.cpp
	src/bstone_cvalidator.h
	src/bstone_cvar.cpp
	src/bstone_cvar.h
	src/bstone_cvar_mgr.cpp
	src/bstone_cvar_mgr.h
	src/bstone_cvar_string.cpp
	src/bstone_cvar_string.h
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_CVAR_SOURCES})
source_group("bstone/cvar" FILES ${BSTONE_BSTONE_CVAR_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_SOURCES
	src/bstone_archiver.cpp
	src/bstone_archiver.h
	src/bstone_atomic_flag.cpp
	src/bstone_atomic_flag.h
	src/bstone_char_hasher.h
	src/bstone_cl.cpp
	src/bstone_cl.h
	src/bstone_configurations.cpp
	src/bstone_door.h
	src/bstone_exception_utils.cpp
	src/bstone_exception_utils.h
	src/bstone_extent_2d.cpp
	src/bstone_extent_2d.h
	src/bstone_fs_utils.cpp
	src/bstone_fs_utils.h
	src/bstone_game_timer.cpp
	src/bstone_game_timer.h
	src/bstone_globals.cpp
	src/bstone_globals.h
	src/bstone_logger.cpp
	src/bstone_logger.h
	src/bstone_math.h
	src/bstone_mt_task_mgr.cpp
	src/bstone_mt_task_mgr.h
	src/bstone_page_mgr.cpp
	src/bstone_page_mgr.h
	src/bstone_precompiled.h
	src/bstone_ref_values.h
	src/bstone_rgb8.cpp
	src/bstone_rgb8.h
	src/bstone_rlew_decoder.cpp
	src/bstone_rlew_decoder.h
	src/bstone_rgb_palette.h
	src/bstone_saved_game.cpp
	src/bstone_saved_game.h
	src/bstone_string_helper.cpp
	src/bstone_string_helper.h
	src/bstone_string_view_hasher.h
	src/bstone_text_reader.cpp
	src/bstone_text_reader.h
	src/bstone_text_writer.cpp
	src/bstone_text_writer.h
	src/bstone_time.cpp
	src/bstone_time.h
	src/bstone_version.cpp
	src/bstone_version.h
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_SOURCES})
source_group("bstone" FILES ${BSTONE_BSTONE_SOURCES})

# ---------------------------------------------------------------------------

# Native Nintendo 3DS components.
target_sources(bstone PRIVATE
    src/bstone_3ds_automap.cpp
    src/bstone_3ds_messages.cpp
    src/bstone_3ds_intro.cpp
    src/bstone_3ds_hud.cpp
)

# The default build produces bstone.elf and bstone.3dsx side by side.
ctr_create_3dsx(bstone NOSMDH)
