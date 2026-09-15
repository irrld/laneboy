cmake_minimum_required(VERSION 3.28)
project(gameboy_emu)

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_BUILD_TYPE Debug)
#add_definitions(-DDEBUG=1)

find_package(fmt CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

add_executable(gameboy_emu
        src/main.cc
        src/register.cc
        src/memory.cc
        src/alu.cc
        src/cpu.cc
        src/util.cc
        src/debug.cc
        src/instructions.cc
        src/cartridge.cc
        src/ppu.cc
        src/emulator.cc
        src/window.cc
        src/renderer.cc
        src/tinyfiledialogs.c
)
include_directories(src)
if(NOT MSVC)
    target_compile_options(gameboy_emu PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-frtti>)
endif()
#target_compile_options(gameboy_emu PRIVATE -frtti -fsanitize=address)
#target_link_options(gameboy_emu PRIVATE -fsanitize=address)

set(IMGUI_DIR "vendor/imgui")
include_directories(${IMGUI_DIR})
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

set(GBIT_DIR "vendor/gbit")
include_directories(${GBIT_DIR})
file(GLOB GBIT_SOURCES ${GBIT_DIR}/lib/*.c)
add_library(gbit
        ${GBIT_SOURCES}
)

target_link_libraries(gameboy_emu
        fmt::fmt
        imgui
        gbit
)

# tinyfiledialogs talks to the common dialog and shell APIs directly.
if(WIN32)
    target_link_libraries(gameboy_emu comdlg32 ole32 shell32 user32)
endif()

install(TARGETS gameboy_emu RUNTIME DESTINATION .)
