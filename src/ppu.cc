#include "ppu.h"

PPU::PPU(CPU& cpu, MemoryBus& bus) : cpu_(cpu), bus_(bus) {
  vram_select_ = 0;
  vram_md_ = std::make_unique<SwitchingMemoryDevice<VRAM_SIZE>>(VRAM_START_ADDRESS, vram_0_, true, true);
  vram_select_md_ = std::make_unique<CallbackOnWriteMemoryDevice>(&vram_select_, [this](u16 address, u8 previous, u8 value){
    if (previous == value) {
      return;
    }
    std::cout << "vram select: " << ToHex(previous) << " -> " << ToHex(value) << std::endl;
    if (value == 0) {
      vram_md_->Switch(vram_0_);
    } else if (value == 1) {
      vram_md_->Switch(vram_1_);
    } else {
      vram_select_ = previous;
    }
  });
  bus_.AddDevice(VRAM_BANK_SELECT_ADDRESS, vram_select_md_.get());
}

void PPU::Step() {

}

void PPU::SetClockSpeed(u32 clock_speed) {
  clock_speed_ = clock_speed;
}