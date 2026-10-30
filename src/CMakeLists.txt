cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)

add_subdirectory(lib/khronos)
add_subdirectory(lib/nuked_opl3)
add_subdirectory(lib/openal_soft)
add_subdirectory(lib/stb)
if (NOT CMAKE_SYSTEM_NAME STREQUAL "Nintendo3DS")
    add_subdirectory(lib/vulkan)
endif ()

if (BSTONE_INTERNAL_SDL2)
	add_subdirectory(lib/sdl2)
endif ()

if (NOT CMAKE_SYSTEM_NAME STREQUAL "Nintendo3DS")
    add_subdirectory(tools)
endif ()
add_subdirectory(bstone)

if (BSTONE_TESTS)
	add_subdirectory(tests)
endif ()