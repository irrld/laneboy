#pragma once

#include <limits>
#include <iostream>
#include <cassert>
#include <bitset>
#include <vector>
#include <string>
#include <sstream>
#include <fmt/core.h>
#include <iomanip>

#define VRAM_START_ADDRESS 0x8000
#define VRAM_SIZE 0x2000
#define VRAM_BANK_SELECT_ADDRESS 0xFF4F

#define WRAM_0_START_ADDRESS 0xC000
#define WRAM_1_7_START_ADDRESS 0xD000
#define WRAM_SIZE 0x2000
#define WRAM_BANK_SELECT_ADDRESS 0xFF70

#define OAM_SIZE 0x0100
#define HRAM_SIZE 0x0080

#define BOOT_UNMAP_ADDRESS 0xFF50
#define INTERRUPT_ENABLE_ADDRESS 0xFFFF
#define INTERRUPT_FLAG_ADDRESS 0xFF0F

// Timer stuff
#define DIV_ADDRESS 0xFF04
#define TIMA_ADDRESS 0xFF05
#define TMA_ADDRESS 0xFF06
#define TAC_ADDRESS 0xFF07

#define BASE_CPU_CLOCK_SPEED 4194304
#define DOUBLE_CPU_CLOCK_SPEED BASE_CPU_CLOCK_SPEED * 2
#define BASE_PPU_CLOCK_SPEED BASE_CPU_CLOCK_SPEED * 4

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;

std::vector<u8> LoadBin(const std::string& path);

inline const char* BoolToStr(bool b) {
  return b ? "true" : "false";
}

inline s8 AsSigned(u8 val) {
  // here the static cast is not required but here to silence the warnings
  if (val & (1 << 7)) { // If the high bit is set
    // Compute two's complement and return
    return static_cast<s8>(-(static_cast<s8>((~val & 0xFF) + 1)));
  }
  // If the high bit is not set
  return static_cast<s8>(val);
}

template<typename Number, int size = sizeof(Number) * 8>
inline std::string ToBinary(Number v) {
  std::stringstream stream;
  std::bitset<size> x(v);
  stream << "0b" << x;
  return stream.str();
}

template<typename Number, int size = sizeof(Number) * 2>
inline std::string ToHex(Number v) {
  std::stringstream stream;
  if (!std::is_unsigned<Number>() && v < 0) {
    v = -v;
    stream << "-";
  }
  stream << "0x";
  stream << std::setw(size)
         << std::setfill('0')
         << std::hex << std::uppercase << (uint64_t) v;
  return stream.str();
}

template<typename Number>
struct AddResult {
  Number value;
  bool did_overflow;
};