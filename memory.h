#pragma once

#include "util.h"

class MemoryBus {
 public:
  MemoryBus();

  u8 Read8(u16 address);
  u16 Read16(u16 address);

  void Write8(u16 address, u8 value);
  void Write16(u16 address, u16 value);

  bool SetNoUninitializedRAM(bool no_uninitialized_ram);
 private:
  friend class CPU;

  u8 mem_[0x10000]{};
  u8 boot_mem_[0x10000]{};
  bool boot_map_[0x10000]{};
  bool init_map_[0x10000]{};
  bool no_uninitialized_ram_ = true;

};