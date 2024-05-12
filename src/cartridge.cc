#include "cartridge.h"
#include "debug.h"

u32 DetermineROMBankNumber(ROMSize rom_size) {
  switch (rom_size) {
    case kRom32KB: return 2;
    case kRom64KB: return 4;
    case kRom128KB: return 8;
    case kRom256KB: return 16;
    case kRom512KB: return 32;
    case kRom1MB: return 64;
    case kRom2MB: return 128;
    case kRom4MB: return 256;
    case kRom8MB: return 512;
    default: return -1;
  }
}

u32 DetermineRAMBankNumber(RAMSize rom_size) {
  switch (rom_size) {
    case kNoRAM: return 0;
    case k1Bank: return 1;
    case k4Banks: return 4;
    case k16Banks: return 16;
    case k8Banks: return 8;
    default: return -1;
  }
}

Cartridge::Cartridge(const std::string& path) {
  data_ = LoadBin(path);
  if (data_.empty()) {
    is_valid_ = false;
    return;
  }
  u8 cgbflag = data_[0x143] & 0b11011111;
  if (cgbflag == 0b10000000) {
    compatibility_ = CartridgeCompatibility::Both;
  } else if (cgbflag == 0b11000000) {
    compatibility_ = CartridgeCompatibility::OnlyGBC;
  } else {
    compatibility_ = CartridgeCompatibility::OnlyDMG;
  }
  type_ = (CartridgeType) data_[0x0147];
  std::cout << "cartridge type: " << (u64)type_ << std::endl;

  rom_size_ = (ROMSize) data_[0x148];
  rom_bank_num_ = DetermineROMBankNumber(rom_size_);
  ram_size_ = (RAMSize) data_[0x149];
  ram_bank_num_ = DetermineRAMBankNumber(ram_size_);
  if (rom_bank_num_ == -1 || ram_bank_num_ == -1) {
    is_valid_ = false;
    return;
  }
  std::cout << "rom banks: " << rom_bank_num_ << std::endl;
  std::cout << "ram banks: " << ram_bank_num_ << std::endl;

  // initialize and load roms
  u32 current = 0x0000;
  for (int i = 0; i < rom_bank_num_; ++i) {
    rom_banks_.emplace_back();
    rom_banks_[i].fill(0);
    memcpy(rom_banks_[i].data(), data_.data() + current, CARTRIDGE_ROM_SIZE);
    current += CARTRIDGE_ROM_SIZE;
  }
  std::cout << "unread data left: " << data_.size() - current << std::endl;
  // initialize rams
  for (int i = 0; i < ram_size_; ++i) {
    ram_banks_.emplace_back();
  }
  ram_bank_select_ = 0;
  ram_bank_md_ = std::make_unique<SwitchingArrayMemoryDevice<CARTRIDGE_RAM_SIZE>>(CARTRIDGE_RAM_START_ADDRESS, &ram_banks_[0], kMemoryAccessBoth);
  rom_bank_00_md_ = std::make_unique<SwitchingArrayWithHandlerMemoryDevice<CARTRIDGE_ROM_SIZE>>(CARTRIDGE_ROM_00_START_ADDRESS, &rom_banks_[0], [this](u16 address,u8 previous, u8 value, bool failed) -> bool {
    //std::cout << "written rom bank 00: " << ToHex(address) << ", " << ToHex(value) << std::endl;
    if (address <= 0x1FFF) { // RAM Enable
      if ((value & 0x0a) == 0x0a) {
        ram_bank_md_->EnableAccess(kMemoryAccessBoth);
        //std::cout << "ram enable" << std::endl;
      } else {
        ram_bank_md_->DisableAccess(kMemoryAccessBoth);
        //std::cout << "ram disable" << std::endl;
      }
    } else if (address >= 0x2000 && address <= 0x3FFF) { // ROM Bank Number
      //std::cout << "written " << ToHex(value) << " to rom select." << std::endl;
      u16 new_value = (rom_bank_select_ & 0b11100000) + ((value & 0b00011111) & (rom_bank_num_ - 1));
      if (new_value == 0) {
        new_value = 1;
      }
      if (rom_bank_select_ == new_value) {
        return previous;
      }
      rom_bank_select_ = new_value;
      rom_bank_01_md_->Switch(&rom_banks_[rom_bank_select_]);
      //std::cout << "rom bank select: " << ToHex(rom_bank_select_) << std::endl;
      EMIT_BANK_CHANGE(*bus_);
    }
    return previous;
  }, kMemoryAccessRead);
  rom_bank_select_ = 1;
  rom_bank_01_md_ = std::make_unique<SwitchingArrayWithHandlerMemoryDevice<CARTRIDGE_ROM_SIZE>>(CARTRIDGE_ROM_01_START_ADDRESS, &rom_banks_[rom_bank_select_], [this](u16 address,u8 previous, u8 value, bool failed) -> bool {
    //std::cout << "written rom bank 01: " << ToHex(address) << ", " << ToHex(value) << std::endl;
    if (address >= 0xA000 && address <= 0xBFFF) {
      //std::cout << "written " << ToHex(value) << " to ram select." << std::endl;
      ram_bank_select_ = value & 0x03;
      ram_bank_md_->Switch(&ram_banks_[ram_bank_select_]);
      //std::cout << "ram bank select: " << ToHex(ram_bank_select_) << std::endl;
      EMIT_BANK_CHANGE(*bus_);
    } else if (address >= 0x4000 && address <= 0x5FFF) {
    } else if (address >= 0x6000 && address <= 0x7FFF) {
    }
    return previous;
  }, kMemoryAccessRead);
  is_valid_ = true;
}

void Cartridge::InitBus(MemoryBus& bus) {
  bus_ = &bus;
  bus.AddDevice(CARTRIDGE_ROM_00_START_ADDRESS, CARTRIDGE_ROM_00_END_ADDRESS, rom_bank_00_md_.get());
  if (rom_bank_num_ > 1) {
    bus.AddDevice(CARTRIDGE_ROM_01_START_ADDRESS, CARTRIDGE_ROM_01_END_ADDRESS, rom_bank_01_md_.get());
  }
  if (ram_bank_num_ > 0) {
    bus.AddDevice(CARTRIDGE_RAM_START_ADDRESS, CARTRIDGE_RAM_END_ADDRESS, ram_bank_md_.get());
  }
  EMIT_BANK_CHANGE(*bus_);
}