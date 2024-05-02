#pragma once

#include <limits>
#include <iostream>
#include <cassert>
#include <bitset>

using u8 = uint8_t;
using s8 = int8_t;
using u16 = uint16_t;
using s16 = int16_t;

inline const char* bool_to_str(bool b) {
  return b ? "true" : "false";
}

inline s8 as_signed(u8 val) {
  // here the static cast is not required but here to silence the warnings
  if (val & (1 << 7)) { // If the high bit is set
    // Compute two's complement and return
    return static_cast<s8>(-(static_cast<s8>((~val & 0xFF) + 1)));
  }
  // If the high bit is not set
  return static_cast<s8>(val);
}

template<typename Number, int size = sizeof(Number) * 8>
inline auto number_to_binary(Number v) {
  std::bitset<size> x(v);
  return x;
}

template<typename Number>
struct AddResult {
  Number value;
  bool did_overflow;
};

template<typename Number>
inline AddResult<Number> add_overflow(Number a, Number b) {
  Number result;
  bool overflow = __builtin_add_overflow(a, b, &result);
  return {result, overflow};
}