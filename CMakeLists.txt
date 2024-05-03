cmake_minimum_required(VERSION 3.28)
project(gameboy_emu)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)
add_definitions(-DDEBUG=1)

add_executable(gameboy_emu
        main.cc
        register.cc
        memory.cc
        cpu.cc
        util.cc
        debug.cc
        instructions.cc
)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(gameboy_emu fmt::fmt)
