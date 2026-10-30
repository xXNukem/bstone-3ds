project(bstone_lib_nuked_opl3 VERSION 1.0.0 LANGUAGES CXX)

add_library(bstone_lib_nuked_opl3 STATIC src/opl3.cpp)
add_library(bstone::lib::nuked_opl3 ALIAS bstone_lib_nuked_opl3)

set_target_properties(bstone_lib_nuked_opl3 PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_compile_definitions(bstone_lib_nuked_opl3 PRIVATE
    $<$<BOOL:${BSTONE_NO_ASSERT}>:BSTONE_NO_ASSERT>
)

if (BSTONE_MORE_COMPILER_WARNINGS)
    target_compile_options(bstone_lib_nuked_opl3 PRIVATE -Wall -Wextra -pedantic)
endif ()

target_include_directories(bstone_lib_nuked_opl3 PUBLIC include)
