#include "memory.h"
#include "debug.h"

MemoryBus::MemoryBus() {
  memset(boot_map_, false, 0x10000);
  memset(init_map_, false, 0x10000);
}

u8 MemoryBus::Read8(u16 address) {
  u8 value;
  if (boot_map_[address]) {
    value = boot_mem_[address];
    EMIT_ROM_READ(address, value);
  } else {
    assert(!no_uninitialized_ram_ || init_map_[address]);
    value = mem_[address];
    EMIT_MEM_READ(address, value);
  }
  return value;
}

u16 MemoryBus::Read16(u16 address) {
  return (((u16) Read8(address + 1) << 8) | Read8(address));
}

void MemoryBus::Write8(u16 address, u8 value) {
  if (address == 0xFF50 && value != 0x00) {
    memset(boot_map_, false, 0x10000);
    std::cout << "unmapped bootloader" << std::endl;
    EMIT_ROM_UNMAP();
  }
  if (boot_map_[address]) {
    return;
  }
  EMIT_MEM_WRITE(address, mem_[address], value);
  mem_[address] = value;
  init_map_[address] = true;
}

void MemoryBus::Write16(u16 address, u16 value) {
  // little endian
  Write8(address, value & 0xFF);
  Write8(address + 1, value >> 8);
}

bool MemoryBus::SetNoUninitializedRAM(bool no_uninitialized_ram) {
  no_uninitialized_ram_ = no_uninitialized_ram;
}