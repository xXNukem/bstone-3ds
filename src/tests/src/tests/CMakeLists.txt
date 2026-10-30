project(bstone_tests VERSION 1.0.0 LANGUAGES CXX)

message(STATUS "[bstone_tests] v${PROJECT_VERSION}")

add_executable(bstone_tests "")

set_target_properties(bstone_tests PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

target_compile_options(bstone_tests
	PRIVATE
        -Werror=return-type
		-Wall -Wextra -pedantic
)

target_include_directories(bstone_tests
	PRIVATE
		${PROJECT_SOURCE_DIR}/../../../bstone/src
)

target_sources(bstone_tests
	PRIVATE
		../../../bstone/src/bstone_char_traits.h
		../../../bstone/src/bstone_exception.cpp
		../../../bstone/src/bstone_exception.h
		../../../bstone/src/bstone_source_location.cpp
		../../../bstone/src/bstone_source_location.h
		../../../bstone/src/bstone_span.h
		../../../bstone/src/bstone_utility.h
		../../../bstone/src/bstone_algorithm.h
		../../../bstone/src/bstone_ascii.h
		../../../bstone/src/bstone_endian.h
		../../../bstone/src/bstone_memory.h
		../../../bstone/src/bstone_memory_resource.cpp
		../../../bstone/src/bstone_memory_resource.h
		../../../bstone/src/bstone_scope_exit.h
		../../../bstone/src/bstone_array.h
		../../../bstone/src/bstone_char_conv.cpp
		../../../bstone/src/bstone_char_conv.cpp
		../../../bstone/src/bstone_sha1.cpp
		../../../bstone/src/bstone_sha1.h
		../../../bstone/src/bstone_single_pool_resource.h
		../../../bstone/src/bstone_auto_arena_resource.cpp
		../../../bstone/src/bstone_auto_arena_resource.h
		../../../bstone/src/bstone_ts_auto_arena_resource.cpp
		../../../bstone/src/bstone_ts_auto_arena_resource.h
		../../../bstone/src/bstone_file_posix.cpp
		../../../bstone/src/bstone_file.cpp
		../../../bstone/src/bstone_file.h
		../../../bstone/src/bstone_stream.cpp
		../../../bstone/src/bstone_stream.h
		../../../bstone/src/bstone_file_stream.cpp
		../../../bstone/src/bstone_file_stream.h
		../../../bstone/src/bstone_memory_stream.cpp
		../../../bstone/src/bstone_memory_stream.h
		../../../bstone/src/bstone_static_ro_memory_stream.cpp
		../../../bstone/src/bstone_static_ro_memory_stream.h
		../../../bstone/src/bstone_crc32.cpp
		../../../bstone/src/bstone_crc32.h
		../../../bstone/src/bstone_api_string.h
		../../../bstone/src/bstone_cxx.h
		../../../bstone/src/bstone_string_view.h
		../../../bstone/src/bstone_fs_posix.cpp
		../../../bstone/src/bstone_fs.h
		../../../bstone/src/bstone_posix_string.cpp
		../../../bstone/src/bstone_posix_string.h
		../../../bstone/src/bstone_four_cc.h
	PRIVATE
		src/bstone_tests_algorithm.cpp
		src/bstone_tests_array.cpp
		src/bstone_tests_ascii.cpp
		src/bstone_tests_char_conv.cpp
		src/bstone_tests_char_traits.cpp
		src/bstone_tests_endian.cpp
		src/bstone_tests_exception.cpp
		src/bstone_tests_memory.cpp
		src/bstone_tests_memory_resource.cpp
		src/bstone_tests_scope_exit.cpp
		src/bstone_tests_source_location.cpp
		src/bstone_tests_span.cpp
		src/bstone_tests_utility.cpp
		src/bstone_tests_sha1.cpp
		src/bstone_tests_single_pool_resource.cpp
		src/bstone_tests_auto_arena_resource.cpp
		src/bstone_tests_ts_auto_arena_resource.cpp
		src/bstone_tests_configurations.cpp
		src/bstone_tests_file.cpp
		src/bstone_tests_file_stream.cpp
		src/bstone_tests_memory_stream.cpp
		src/bstone_tests_static_ro_memory_stream.cpp
		src/bstone_tests_crc32.cpp
		src/bstone_tests_api_string.cpp
		src/bstone_tests_string_view.cpp
		src/bstone_tests_fs.cpp
		src/bstone_tests_four_cc.cpp
	PRIVATE
		src/bstone_tester.cpp
		src/bstone_tester.h
		src/bstone_tests.cpp
)

ctr_create_3dsx(bstone_tests NOSMDH)
