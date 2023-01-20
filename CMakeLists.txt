# CMakeList.txt : CMake project for omcmeas, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)

project("nbt")

set(CMAKE_CXX_STANDARD 23)
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)


# --------------------------------------------------------------------------------------------------
# -- library for usage in other projects -----------------------------------------------------------
# --------------------------------------------------------------------------------------------------

include_directories(src)
include_directories(include)

add_library (nbtlib
    STATIC
    "src/nbt.cpp"
    )

target_link_libraries(nbtlib PRIVATE
		spdlog::spdlog
		ZLIB::ZLIB)
