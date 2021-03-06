cmake_minimum_required(VERSION 3.16)

project(chip8 VERSION 0.1)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/sdl2_cmake_files)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CSS_STANDARD_REQUIRED True)
set(CMAKE_DEBUG_POSTFIX debug)

add_executable(chip8 main.cpp)
set_target_properties(chip8 PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

find_package(SDL2 REQUIRED)
target_link_libraries(chip8 SDL2::Core)

find_package(SDL2_mixer REQUIRED)
target_link_libraries(chip8 SDL2::Mixer)

configure_file(version.h.in version.h)

target_include_directories(chip8 PUBLIC "${PROJECT_BINARY_DIR}")