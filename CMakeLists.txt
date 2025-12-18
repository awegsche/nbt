# CMakeList.txt : CMake project for omcmeas, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)

project("nbt" VERSION 1.0.0 )

find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)
find_package(spdlog CONFIG REQUIRED)


# --------------------------------------------------------------------------------------------------
# -- library for usage in other projects -----------------------------------------------------------
# --------------------------------------------------------------------------------------------------

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library (nbtlib
    STATIC
    "src/nbt.cpp"
    "src/region.cpp"
    )

target_include_directories(nbtlib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE src)

target_link_libraries(nbtlib PRIVATE
		spdlog::spdlog
		ZLIB::ZLIB)

set_target_properties(nbtlib PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1)

install(TARGETS nbtlib EXPORT NbtLibConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install (DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nbtlib)

install(EXPORT NbtLibConfig DESTINATION share/NbtLib/cmake)
export(TARGETS nbtlib FILE NbtLibConfig.cmake)

# TESTS
include(FetchContent)
FetchContent_Declare(
		googletest
		URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(test_primitives
		tests/test_primitives.cpp)

target_link_libraries(
		test_primitives
		GTest::gtest_main
		spdlog::spdlog
		ZLIB::ZLIB
		nbtlib
)

add_executable(test_io
		tests/test_io.cpp)

target_link_libraries(
		test_io
		GTest::gtest_main
		spdlog::spdlog
		ZLIB::ZLIB
		nbtlib
)

add_executable(unit_tests
		tests/unit_tests.cpp)

target_link_libraries(
		unit_tests
		GTest::gtest_main
		spdlog::spdlog
		ZLIB::ZLIB
		nbtlib
)

add_executable(test_list
		tests/test_list.cpp)

target_link_libraries(
		test_list
		GTest::gtest_main
		spdlog::spdlog
		ZLIB::ZLIB
		nbtlib
)

add_executable(test_endian
		tests/test_endian.cpp)

target_link_libraries(
		test_endian
		GTest::gtest_main
		spdlog::spdlog
		ZLIB::ZLIB
		nbtlib
)

add_executable(test_gzip
		tests/test_gzip.cpp)

target_link_libraries(
		test_gzip
		GTest::gtest_main
		spdlog::spdlog
		ZLIB::ZLIB
		nbtlib
)

add_executable(test_region
		tests/test_region.cpp)

target_link_libraries(
		test_region
		GTest::gtest_main
		spdlog::spdlog
		ZLIB::ZLIB
		nbtlib
)

include(GoogleTest)
gtest_discover_tests(test_primitives)
gtest_discover_tests(test_io)
gtest_discover_tests(unit_tests)
gtest_discover_tests(test_list)
gtest_discover_tests(test_endian)
gtest_discover_tests(test_gzip)
gtest_discover_tests(test_region)
