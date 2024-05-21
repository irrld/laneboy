#pragma once

#include "alu.h"
#include "cartridge.h"
#include "event.h"
#include "memory.h"
#include "register.h"

inline std::string InterruptTypeToString(InterruptType type) {
  switch (type) {
    case kInterruptTypeVBlank:
      return "VBlank";
    case kInterruptTypeLCDStat:
      return "LCDStat";
    case kInterruptTypeTimer:
      return "Timer";
    case kInterruptTypeSerial:
      return "Serial";
    case kInterruptTypeJoypad:
      return "Joypad";
    case kInterruptMax:
      return "InterruptMax";
  }
}

class CPU {
 public:
  CPU(EventBus& event_bus, MemoryBus& bus);
  CPU(const CPU&) = delete;

  // Control
  void Stop();
  void Halt();
  void Step();
  void HandleInterrupts();

  void UpdateTimers(u32 cycles);
  void SetClockFrequency();
  void UpdateDivider(u32 cycles);
  bool IsTimerEnabled();

  void EnableInterrupt(InterruptType type);
  void DisableInterrupt(InterruptType type);
  void SendInterrupt(InterruptType type);
  void ClearInterrupt(InterruptType type);
  void SetInterruptMasterEnable(bool enable, bool immediate = false);

  void SetClockSpeed(u32 clock_speed);

  void Push(u16 value);
  u16 Pop();

  void StartDMA(u8 value);
  void ProcessDMA();

  void OnEvent(Event& event);

  // Load
  void LoadBootRom(std::vector<u8> data);
  void UnloadBootRom();
  void LoadCartridge(std::unique_ptr<Cartridge> cartridge);

 public:
  Registers registers_;
  EventBus& event_bus_;
  MemoryBus& bus_;
  ALU alu{registers_};

  std::unique_ptr<Cartridge> cartridge_;
  std::unique_ptr<MemoryDevice> rom_device_;
  u16 rom_size_;

  bool running_ = false;
  bool halted_ = false;
  u32 clock_speed_ = 0; // in T-cycles
  u32 cycles_consumed_ = 0;
  u32 ic_ = 0; // instruction counter
  u8 boot_unloaded_ = true; // not loaded by default
  std::unique_ptr<MemoryDevice> boot_unmap_md_;

  // todo double speed mode switching

  // Interrupt
  bool ime_ = false;
  bool ime_pending_ = false;
  u8 ie_ = 0;

  u8 if_ = 0;
  // Timer
  u8 div_;
  u8 tima_ = 0;
  u8 tma_ = 0;
  u8 tac_ = 0;
  u32 tic_ = 0;

  s32 timer_clock_ = 0;
  s32 div_clock_ = 0;
  bool tima_overflow_ = false;

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
  std::unique_ptr<MemoryDevice> wram_0_md_; // switches 1-7
  std::unique_ptr<SwitchingArrayMemoryDevice<WRAM_SIZE>> wram_1_7_md_; // switches 1-7
  std::unique_ptr<MemoryDevice> wram_select_md_;

  // Other regions
  std::array<u8, OAM_SIZE> oam_;
  std::unique_ptr<MemoryDevice> oam_md_;

  std::array<u8, HRAM_SIZE> hram_;
  std::unique_ptr<MemoryDevice> hram_md_;

  // Audio Stuff
  std::array<u8, AUDIO_SIZE> audio_;
  std::unique_ptr<MemoryDevice> audio_md_;

  std::array<u8, WAVE_PATTERN_SIZE> wave_pattern_;
  std::unique_ptr<MemoryDevice> wave_pattern_md_;

  // Video RAM (8KiB each)
  u8 vram_select_;
  std::array<u8, VRAM_SIZE> vram_0_;
  std::array<u8, VRAM_SIZE> vram_1_;
  std::unique_ptr<SwitchingArrayMemoryDevice<VRAM_SIZE>> vram_md_; // switches 0-1, only in CGB mode
  std::unique_ptr<MemoryDevice> vram_select_md_;

  u8 ly_; // read only
  std::unique_ptr<MemoryDevice> ly_md_;

  u8 lyc_;
  std::unique_ptr<MemoryDevice> lyc_md_; // read/write

  u8 obp0_;
  std::unique_ptr<MemoryDevice> obp0_md_;

  u8 obp1_;
  std::unique_ptr<MemoryDevice> obp1_md_;

  LCDC lcdc_;
  std::unique_ptr<MemoryDevice> lcdc_md_;

  LCDS lcds_;
  std::unique_ptr<MemoryDevice> lcds_md_;

  // only in DMG mode
  u8 bgp_;
  std::unique_ptr<MemoryDevice> bgp_md_;


  // only in DMG
  u8 bcps_;
  // only in CGB
  u8 bgpi_;
  std::unique_ptr<MemoryDevice> bcps_bgpi_md_;

  // only in DMG
  u8 ocps_;
  // only in CGB
  u8 obpi_;
  std::unique_ptr<MemoryDevice> ocps_obpi_md_;

  // only in DMG
  u8 ocpd_;
  // only in CGB
  u8 obpd_;
  std::unique_ptr<MemoryDevice> ocpd_obpd_md_;

  u8 scx_;
  std::unique_ptr<MemoryDevice> scx_md_;

  u8 scy_;
  std::unique_ptr<MemoryDevice> scy_md_;

  u8 wx_;
  std::unique_ptr<MemoryDevice> wx_md_;

  u8 wy_;
  std::unique_ptr<MemoryDevice> wy_md_;

  u8 joyp_;
  std::unique_ptr<MemoryDevice> joyp_md_;

  CPUMode cpu_mode_;
  std::unique_ptr<MemoryDevice> cpu_mode_md_;
  u8 cpu_mode_lock_;
  std::unique_ptr<MemoryDevice> cpu_mode_lock_md_;

  u8 dma_;
  s16 dma_current_;
  std::unique_ptr<MemoryDevice> dma_md_;

  // Timer and interrupt memory devices for mapping
  std::unique_ptr<MemoryDevice> ie_md_;
  std::unique_ptr<MemoryDevice> if_md_;
  std::unique_ptr<MemoryDevice> div_md_;
  std::unique_ptr<MemoryDevice> tima_md_;
  std::unique_ptr<MemoryDevice> tma_md_;
  std::unique_ptr<MemoryDevice> tac_md_;

  // Serial
  u8 sb_;
  std::unique_ptr<MemoryDevice> sb_md_;
  u8 sc_;
  std::unique_ptr<MemoryDevice> sc_md_;

  // Prepare speed switch
  u8 key1_;
  std::unique_ptr<MemoryDevice> key1_md_;

};