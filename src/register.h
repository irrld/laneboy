#pragma once

#include "util.h"

enum class ArithmeticTarget {
  A,
  B,
  C,
  D,
  E,
  H,
  L,
  BC,
  DE,
  HL,
  SP
};

bool is_16bit_register(ArithmeticTarget target);

union Flags {
  u8 v;
  struct {
    bool carry : 1;
    bool half_carry : 1;
    bool subtract : 1;
    bool zero : 1;
  } f;

  Flags(u8 b) {
    v = b;
  }
};

union Register {
  u16 v;
  struct {
    u8 lo;
    u8 hi;
  } f;
};

struct Registers {
  u8 a = 0; // a
  Flags flags{0};
  Register bc{0}; // b, c
  Register de{0}; // d, e
  Register hl{0}; // h, l
  u16 sp = 0;
  u16 pc = 0;

  void Set(ArithmeticTarget target, u16 v);
  u16 Get(ArithmeticTarget target) const;
  void Print();
};
