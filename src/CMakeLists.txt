# CMakeList.txt : CMake project for omcmeas, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)

project("nbt")

set(CMAKE_CXX_STANDARD 23)
find_package(Threads REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(ZLIB REQUIRED)


# --------------------------------------------------------------------------------------------------
# -- library for usage in other projects -----------------------------------------------------------
# --------------------------------------------------------------------------------------------------

add_library (nbtlib
    STATIC
    "nbt.cpp"
    )

target_link_libraries(nbtlib PRIVATE
		spdlog::spdlog
		ZLIB::ZLIB)

# --------------------------------------------------------------------------------------------------
# ---- executable for testing ----------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------

add_executable (nbttest
    "main.cpp"
    "nbt.cpp"
)

target_link_options(nbttest PRIVATE -static-libgcc -static-libstdc++)

target_link_libraries(nbttest PRIVATE
		spdlog::spdlog
		ZLIB::ZLIB)

add_custom_target(run
    COMMAND nbttest
    DEPENDS nbttest
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
)
