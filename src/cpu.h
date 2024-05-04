#pragma once

#include "cartridge.h"
#include "memory.h"
#include "register.h"

//V-Blank: 0x40
//LCD STAT: 0x48
//Timer: 0x50
//Serial: 0x58
//Joypad: 0x60
enum class InterruptType : u8 {
  VBlank = 0b0000'0001,
  LCDStat = 0b0000'0010,
  Timer = 0b0000'0100,
  Serial = 0b0000'1000,
  Joypad = 0b0001'0000,
  InterruptMax = 6
};

inline std::string InterruptTypeToString(InterruptType type) {
  switch (type) {
    case InterruptType::VBlank:
      return "VBlank";
    case InterruptType::LCDStat:
      return "LCDStat";
    case InterruptType::Timer:
      return "Timer";
    case InterruptType::Serial:
      return "Serial";
    case InterruptType::Joypad:
      return "Joypad";
    case InterruptType::InterruptMax:
      return "InterruptMax";
  }
}

class CPU {
 public:
  CPU(MemoryBus& bus);

  // Control
  void Stop();
  void Halt();
  void Step();
  void HandleInterrupts();

  void UpdateTimers(int cycles);
  void UpdateDividerRegister(int cycles);
  void SetClockFrequency();
  bool IsTimerEnabled();

  void EnableInterrupt(InterruptType type);
  void DisableInterrupt(InterruptType type);
  void SendInterrupt(InterruptType type);
  void ClearInterrupt(InterruptType type);
  void SetInterruptMasterEnable(bool enable);

  void SetClockSpeed(u32 clock_speed);

  void Push(u16 value);
  u16 Pop();

  // Memory
  u8 Fetch8();
  u16 Fetch16();

  // Load
  void LoadBootRom(const u8* bin, u16 size);
  void UnloadBootRom();
  void LoadCartridge(std::unique_ptr<Cartridge> cartridge);

  // ALU
  u8 Add(u8 first, u8 second);
  u8 AddWithCarry(u8 first, u8 second);
  u16 AddWord(u16 first, u16 second);

  u8 Inc(u8 first);
  u16 IncWord(u16 first);

  u8 Sub(u8 first, u8 second);
  u16 SubWord(u16 first, u16 second);
  u8 SubWithCarry(u8 first, u8 second);

  u8 Dec(u8 first);
  u16 DecWord(u16 first);

 public:
  Registers registers_;
  MemoryBus& bus_;
  std::unique_ptr<Cartridge> cartridge_;
  std::unique_ptr<MemoryDevice> rom_device_;
  u16 rom_size_;

  bool running_ = false;
  bool halted_ = false;
  u32 clock_speed_ = 0; // in T-cycles
  u32 current_cycle_ = 0;
  u32 ic_ = 0; // instruction counter
  u8 boot_unloaded_ = true; // not loaded by default
  std::unique_ptr<MemoryDevice> boot_unmap_md_;

  // todo add cgb mode flag and switching
  // todo double speed mode switching

  // Interrupt
  bool ime_ = false;
  u8 ie_ = 0;

  u8 if_ = 0;
  // Timer
  u8 div_ = 0;
  u8 tima_ = 0;
  u8 tma_ = 0;
  u8 tac_ = 0;

  u32 timer_period_ = 0;
  u32 div_period_ = 0;

  // Bank stuff
  // Work RAM (4KiB each)
  u8 wram_select_; // 0x00 is treated as 0x01, only switchable in CGB mode
  std::array<u8, WRAM_SIZE> wram_0_;
  std::array<u8, WRAM_SIZE> wram_1_;
  std::array<u8, WRAM_SIZE> wram_2_;
  std::array<u8, WRAM_SIZE> wram_3_;
  std::array<u8, WRAM_SIZE> wram_4_;
  std::array<u8, WRAM_SIZE> wram_5_;
  std::array<u8, WRAM_SIZE> wram_6_;
  std::array<u8, WRAM_SIZE> wram_7_;
  std::unique_ptr<FixedMemoryDevice<WRAM_SIZE>> wram_0_md_; // switches 1-7
  std::unique_ptr<SwitchingMemoryDevice<WRAM_SIZE>> wram_1_7_md_; // switches 1-7
  std::unique_ptr<MemoryDevice> wram_select_md_;

  // Other regions
  std::array<u8, OAM_SIZE> oam_;

  std::array<u8, HRAM_SIZE> hram_;
  // Timer and interrupt memory devices for mapping
  std::unique_ptr<MemoryDevice> ie_md_;
  std::unique_ptr<MemoryDevice> if_md_;
  std::unique_ptr<MemoryDevice> div_md_;
  std::unique_ptr<MemoryDevice> tima_md_;
  std::unique_ptr<MemoryDevice> tma_md_;
  std::unique_ptr<MemoryDevice> tac_md_;
};