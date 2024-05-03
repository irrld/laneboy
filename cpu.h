#pragma once

#include "register.h"
#include "memory.h"

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

#define INTERRUPT_ENABLE_ADDRESS 0xFFFF
#define INTERRUPT_FLAG_ADDRESS 0xFF0F
// Timer stuff
#define DIV_ADDRESS 0xFF04
#define TIMA_ADDRESS 0xFF05
#define TMA_ADDRESS 0xFF06
#define TAC_ADDRESS 0xFF07

struct CPU {
  Registers registers_;
  MemoryBus bus_;

  bool running_ = false;
  bool halted_ = false;
  u32 clock_speed_ = 0; // in T-cycles
  u32 current_cycle_ = 0;
  u32 ic_ = 0; // instruction counter

  bool ime_ = false;

  // Timer
  u32 timer_period_ = 0;
  u32 div_period_ = 0;

  CPU();

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

  void Push(u16 value);
  u16 Pop();

  // Memory
  u8 Fetch8();
  u16 Fetch16();

  // Load
  void LoadBootRom(const u8* bin, u16 size);
  void LoadRom(const u8* bin, u16 size);

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

};