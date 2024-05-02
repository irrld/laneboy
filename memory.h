#pragma once

#include "util.h"

#define CHECK_OOB(address) assert(address < 0xFFFF && address > 0)

struct MemoryBus {
  u8 memory_[0xFFFF];

  u8 Read8(u16 address) {
    return memory_[address];
  }

  u16 Read16(u16 address) {
    CHECK_OOB(address);
    CHECK_OOB(address + 1);
    return ((u16)memory_[address] << 8) | memory_[address + 1];
  }

  void Write8(u16 address, u8 value) {
    CHECK_OOB(address);
    memory_[address] = value;
  }

  void Write16(u16 address, u16 value) {
    CHECK_OOB(address);
    CHECK_OOB(address + 1);
    memory_[address] = value >> 8;
    memory_[address + 1] = value & 0xFF;
  }
};