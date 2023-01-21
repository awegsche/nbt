# CMakeList.txt : CMake project for omcmeas, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)

project("nbt" VERSION 1.0.0 LANGUAGES CXX)

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

target_include_directories(nbtlib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
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

