cmake_minimum_required(VERSION 3.28)
project(gameboy_emu)

set(CMAKE_CXX_STANDARD 20)

add_executable(gameboy_emu
        main.cc
        register.cc
        memory.cc
        cpu.cc
)
