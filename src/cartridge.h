
#pragma once

#include "util.h"
#include "memory.h"

enum class CartridgeType {
  ROM_ONLY = 0x00,
  MBC1 = 0x01,
  MBC2_RAM = 0x02,
  MBC2_RAM_BATTERY = 0x03,
  MBC2 = 0x05,
  MBC2_BATTERY = 0x06,
  ROM_RAM = 0x08, // not used
  ROM_RAM_BATTERY = 0x09, // not used
  MMM01 = 0x0B,
  MMM01_RAM = 0x0C,
  MMM01_RAM_BATTERY = 0x0D,
  MBC3_TIMER_BATTERY = 0x0F,
  MBC3_TIMER_RAM_BATTERY = 0x10, // not implemented, only used by Pocket Monsters: Crystal Version
  MBC3 = 0x11,
  MBC3_RAM = 0x12, // same as 0x10
  MBC3_RAM_BATTERY = 0x13, // same as 0x10
  MBC5 = 0x19,
  MBC5_RAM = 0x1A,
  MBC5_RAM_BATTERY = 0x1B,
  MBC5_RUMBLE = 0x1C,
  MBC5_RUMBLE_RAM = 0x1D,
  MBC5_RUMBLE_RAM_BATTERY = 0x1E,
  MBC6 = 0x20,
  MBC7_SENSOR_RUMBLE_RAM_BATTERY = 0x20,
  POCKET_CAMERA = 0xFC,
  BANDAI_TAMA5 = 0xFD,
  HUC3 = 0xFE,
  HUC1_RAM_BATTERY = 0xFF,
};

enum RomSize {
  kRom32KB = 0x00,
  kRom64KB = 0x01,
  kRom128KB = 0x02,
  kRom256KB = 0x03,
  kRom512KB = 0x04,
  kRom1MB = 0x05,
  kRom2MB = 0x06,
  kRom4MB = 0x07,
  kRom8MB = 0x08
};

enum RamSize {
  kNoRAM = 0x00,
  // 0x01 is unused, potentially 2KiB
  k1Bank = 0x02,
  k4Banks = 0x03, // 4 banks of 8KiB each
  k16Banks = 0x04, // 16 banks of 8KiB each
  k8Banks = 0x05 // 8 banks of 8KiB each
};

class Cartridge {
 public:
  Cartridge(const std::string& path);

  const std::vector<u8>& data() const { return data_; }

  bool is_valid() const { return is_valid_;}

  void InitBus(MemoryBus& bus);
 private:
  std::vector<u8> data_;
  bool is_valid_;

  RomSize rom_size_;
  u16 rom_bank_num_;
  RamSize ram_size_;
  u16 ram_bank_num_;
};
