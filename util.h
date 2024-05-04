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

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using s8 = int8_t;
using s16 = int16_t;

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