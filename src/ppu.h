#pragma once

#include "util.h"
#include "memory.h"
#include "cpu.h"

class PPU {
 public:
  PPU(CPU& cpu, MemoryBus& bus);

  void Step();
  void SetClockSpeed(u32 clock_speed);

 public:
  CPU& cpu_;
  MemoryBus& bus_;
  u32 clock_speed_;

  // Video RAM (8KiB each)
  u8 vram_select_;
  std::array<u8, VRAM_SIZE> vram_0_;
  std::array<u8, VRAM_SIZE> vram_1_;
  std::unique_ptr<SwitchingMemoryDevice<VRAM_SIZE>> vram_md_; // switches 0-1, only in CGB mode
  std::unique_ptr<MemoryDevice> vram_select_md_;

};