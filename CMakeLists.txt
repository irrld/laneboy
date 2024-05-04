cmake_minimum_required(VERSION 3.28)
project(gameboy_emu)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)
add_definitions(-DDEBUG=1)

add_executable(gameboy_emu
        src/main.cc
        src/register.cc
        src/memory.cc
        src/cpu.cc
        src/util.cc
        src/debug.cc
        src/instructions.cc
        src/cartridge.cc
        src/ppu.cc
        src/emulator.cc
        src/window.cc
        src/renderer.cc
)

set(IMGUI_DIR "vendor/imgui")
include_directories(src ${IMGUI_DIR})
file(GLOB IMGUI_SOURCES ${IMGUI_DIR}/*.cpp)
add_library(imgui
        ${IMGUI_SOURCES}
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_link_libraries(imgui PUBLIC
        OpenGL::GL
        GLEW::GLEW
        glfw
)

find_package(fmt CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW3 REQUIRED)

target_link_libraries(gameboy_emu
        fmt::fmt
        imgui
)
