
#pragma once

#include "util.h"
#include "memory.h"

enum class CartridgeType {
  ROM_ONLY = 0x00,
  MBC1 = 0x01,
  MBC1_RAM = 0x02,
  MBC1_RAM_BATTERY = 0x03,
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

enum ROMSize {
  kRom32KB = 0x00, // 2 banks, no switching
  kRom64KB = 0x01, // 4 banks, 1 is fixed, 3 switching
  kRom128KB = 0x02, // 8 banks, 1 is fixed, 7 switching
  kRom256KB = 0x03, // 16 banks, 1 is fixed, 15 switching
  kRom512KB = 0x04, // 32 banks, 1 is fixed, 31 switching
  kRom1MB = 0x05, // 64 banks, 1 is fixed, 63 switching
  kRom2MB = 0x06, // 128 banks, 1 is fixed, 127 switching
  kRom4MB = 0x07, // 256 banks, 1 is fixed, 255 switching
  kRom8MB = 0x08 // 512 banks, 1 is fixed, 511 switching
};

enum RAMSize {
  kNoRAM = 0x00,
  // 0x01 is unused, potentially 2KiB
  k1Bank = 0x02,
  k4Banks = 0x03, // 4 banks of 8KiB each
  k16Banks = 0x04, // 16 banks of 8KiB each
  k8Banks = 0x05 // 8 banks of 8KiB each
};

enum class CartridgeCompatibility {
  Both,
  OnlyGBC,
  OnlyDMG
};

u32 DetermineROMBankNumber(ROMSize rom_size);
u32 DetermineRAMBankNumber(RAMSize rom_size);

class Cartridge {
 public:
  Cartridge(const std::string& path);

  const std::vector<u8>& data() const { return data_; }

  bool is_valid() const { return is_valid_;}

  void InitBus(MemoryBus& bus);

  CartridgeCompatibility compatibility() const { return compatibility_; }
 private:
  std::vector<u8> data_;
  bool is_valid_;

  MemoryBus* bus_;
  ROMSize rom_size_;
  u32 rom_bank_num_;
  RAMSize ram_size_;
  u32 ram_bank_num_;
  CartridgeCompatibility compatibility_;
  CartridgeType type_;

  u8 rom_bank_select_;
  std::vector<std::array<u8, CARTRIDGE_ROM_SIZE>> rom_banks_;
  std::unique_ptr<SwitchingArrayMemoryDevice<CARTRIDGE_ROM_SIZE>> rom_bank_00_md_;
  std::unique_ptr<SwitchingArrayMemoryDevice<CARTRIDGE_ROM_SIZE>> rom_bank_01_md_;

  u8 ram_bank_select_;
  std::vector<std::array<u8, CARTRIDGE_RAM_SIZE>> ram_banks_;
  std::unique_ptr<SwitchingArrayMemoryDevice<CARTRIDGE_RAM_SIZE>> ram_bank_md_;

};
