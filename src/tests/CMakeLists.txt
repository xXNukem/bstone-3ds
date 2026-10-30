cmake_minimum_required(VERSION 3.18)
project(bstone_component_tests LANGUAGES CXX)

if (NOT CMAKE_SYSTEM_NAME STREQUAL "Nintendo3DS" OR NOT COMMAND ctr_create_3dsx)
    message(FATAL_ERROR "Component tests require the devkitPro Nintendo 3DS toolchain.")
endif ()

add_subdirectory(src/tests)
