cmake_minimum_required(VERSION 3.18)

# Select the devkitPro toolchain before enabling any compiler.
# An explicit -DCMAKE_TOOLCHAIN_FILE takes precedence.
if (NOT CMAKE_TOOLCHAIN_FILE)
    if (NOT DEFINED ENV{DEVKITPRO})
        message(FATAL_ERROR "Set DEVKITPRO or pass -DCMAKE_TOOLCHAIN_FILE=/path/to/devkitpro/cmake/3DS.cmake")
    endif ()
    set(CMAKE_TOOLCHAIN_FILE "$ENV{DEVKITPRO}/cmake/3DS.cmake" CACHE FILEPATH "devkitARM Nintendo 3DS toolchain")
endif ()

project(bstone_solution LANGUAGES C CXX)

if (NOT CMAKE_SYSTEM_NAME STREQUAL "Nintendo3DS" OR NOT COMMAND ctr_create_3dsx)
    message(FATAL_ERROR "BStone 3DS requires the devkitPro Nintendo 3DS toolchain (cmake/3DS.cmake).")
endif ()

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif ()

option(BSTONE_NO_ASSERT "Disable assertions." OFF)
option(BSTONE_MORE_COMPILER_WARNINGS "Enable additional compiler warnings." OFF)
option(BSTONE_USE_PCH "Use precompiled headers." OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if ("${BSTONE_CI_GIT_HASH}" STREQUAL "")
	include(GetGitHeadHash)

	get_git_hash(${CMAKE_SOURCE_DIR} BSTONE_GIT_HASH)
	message(STATUS "[bstone_solution] Source Git hash: ${BSTONE_GIT_HASH}")
else ()
	message(STATUS "[bstone_solution] CI Git hash: ${BSTONE_CI_GIT_HASH}")
	set(BSTONE_GIT_HASH "${BSTONE_CI_GIT_HASH}")
endif ()

if (BSTONE_GIT_HASH)
    string(SUBSTRING "${BSTONE_GIT_HASH}" 0 8 BSTONE_GIT_HASH_SHORT)
endif ()
message(STATUS "[bstone_solution] Short Git hash: ${BSTONE_GIT_HASH_SHORT}")

add_subdirectory(src)
