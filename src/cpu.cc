#include "cpu.h"
#include "cpu_events.h"
#include "debug.h"
#include "instructions.h"

// todo change this to generic memory devices, no need for a custom type
class BootROMDevice : public MemoryDevice {
 public:
  BootROMDevice(std::vector<u8> data) : MemoryDevice(kMemoryAccessRead), data_(std::move(data)) {

  }

  bool CheckAccess(u16 address, MemoryAccess type) override {
      return address < data_.size() && type == kMemoryAccessRead;
  }

  u8 Read(u16 address) override {
    return data_[address];
  }

  void Write(u16 address, u8 value) override {
  }

 private:
  std::vector<u8> data_;
};

CPU::CPU(EventBus& event_bus, MemoryBus& bus) : event_bus_(event_bus), bus_(bus) {
  event_bus_.Subscribe(BIND_FN(OnEvent));

  SetClockSpeed(BASE_CPU_CLOCK_SPEED);

  ie_md_ = std::make_unique<FixedPointerMemoryDevice<1, u8>>(&ie_, kMemoryAccessBoth);
  bus_.AddDevice(INTERRUPT_ENABLE_ADDRESS, ie_md_.get(), true);
  if_md_ = std::make_unique<FixedPointerMemoryDevice<1, u8>>(&if_, kMemoryAccessBoth);
  bus_.AddDevice(INTERRUPT_FLAG_ADDRESS, if_md_.get(), true);
  div_md_ = std::make_unique<FixedPointerWithHandlerMemoryDevice<1, u8>>(&div_, [this](u16 address, u8 previous, u8 value, bool failed) -> u8 {
    return 0x00;
  }, kMemoryAccessBoth);
  bus_.AddDevice(DIV_ADDRESS, div_md_.get(), true);
  tima_ = 0;
  tima_md_ = std::make_unique<FixedPointerMemoryDevice<1, u8>>(&tima_, kMemoryAccessBoth);
  bus_.AddDevice(TIMA_ADDRESS, tima_md_.get(), true);
  tma_ = 0;
  tma_md_ = std::make_unique<FixedPointerMemoryDevice<1, u8>>(&tma_, kMemoryAccessBoth);
  bus_.AddDevice(TMA_ADDRESS, tma_md_.get(), true);
  tac_ = 0;
  tac_md_ = std::make_unique<FixedPointerWithHandlerMemoryDevice<1, u8>>(&tac_, [this](u16 address, u8 previous, u8 value, bool failed){
    if ((previous & 0b11) != (value & 0b11)) {
      SetClockFrequency();
    }
    return value;
  }, kMemoryAccessBoth);
  bus_.AddDevice(TAC_ADDRESS, tac_md_.get(), true);
  boot_unmap_md_ = std::make_unique<FixedPointerWithHandlerMemoryDevice<1, u8>>(&boot_unloaded_, [this](u16 address, u8 previous, u8 value, bool failed){
    if (value != 0) {
      UnloadBootRom();
      return 0xFF;
    }
    return 0x00;
  }, kMemoryAccessBoth);
  bus_.AddDevice(BOOT_UNMAP_ADDRESS, boot_unmap_md_.get(), true);

  // WRAM 0 is fixed
  wram_0_.fill(0);
  wram_0_md_ = std::make_unique<FixedArrayMemoryDevice<WRAM_SIZE>>(WRAM_0_START_ADDRESS, &wram_0_, kMemoryAccessBoth);
  bus_.AddDevice(WRAM_0_START_ADDRESS, WRAM_0_END_ADDRESS, wram_0_md_.get());

  // WRAM 1 is selectable
  wram_1_.fill(0);
  wram_select_ = 0x01;
  wram_1_7_md_ = std::make_unique<SwitchingArrayMemoryDevice<WRAM_SIZE>>(WRAM_1_7_START_ADDRESS, &wram_1_, kMemoryAccessBoth);
  bus_.AddDevice(WRAM_1_7_START_ADDRESS, WRAM_1_7_END_ADDRESS, wram_1_7_md_.get());
  wram_select_md_ = std::make_unique<FixedPointerWithHandlerMemoryDevice<1, u8>>(&wram_select_, [this](u16 address, u8 previous, u8 value, bool failed){
    if (previous == value || cpu_mode_ == kCPUModeDMG) {
      return previous;
    }
    std::cout << "wram select: " << ToHex(previous) << " -> " << ToHex(value) << std::endl;
    switch (value & 0x07) {
      case 0:
        value = 1;
      case 1:
        wram_1_7_md_->Switch(&wram_1_);
        break;
      case 2:
        wram_1_7_md_->Switch(&wram_2_);
        break;
      case 3:
        wram_1_7_md_->Switch(&wram_3_);
        break;
      case 4:
        wram_1_7_md_->Switch(&wram_4_);
        break;
      case 5:
        wram_1_7_md_->Switch(&wram_5_);
        break;
      case 6:
        wram_1_7_md_->Switch(&wram_6_);
        break;
      case 7:
        wram_1_7_md_->Switch(&wram_7_);
        break;
      default:
        value = previous;
        break;
    }
    EMIT_BANK_CHANGE(bus_);
    return value;
  }, kMemoryAccessBoth);
  bus_.AddDevice(WRAM_BANK_SELECT_ADDRESS, wram_select_md_.get());

  oam_md_ = std::make_unique<FixedPointerMemoryDevice<OAM_SIZE, u8>>(OAM_START_ADDRESS, oam_.data(), kMemoryAccessBoth);
  bus_.AddDevice(OAM_START_ADDRESS, OAM_END_ADDRESS, oam_md_.get());

  hram_md_ = std::make_unique<FixedArrayMemoryDevice<HRAM_SIZE>>(HRAM_START_ADDRESS, &hram_, kMemoryAccessBoth);
  bus_.AddDevice(HRAM_START_ADDRESS, HRAM_END_ADDRESS, hram_md_.get());

  audio_md_ = std::make_unique<FixedArrayMemoryDevice<AUDIO_SIZE>>(AUDIO_START_ADDRESS, &audio_, kMemoryAccessBoth);
  bus_.AddDevice(AUDIO_START_ADDRESS, AUDIO_END_ADDRESS, audio_md_.get());

  wave_pattern_md_ = std::make_unique<FixedArrayMemoryDevice<WAVE_PATTERN_SIZE>>(WAVE_PATTERN_START_ADDRESS, &wave_pattern_, kMemoryAccessBoth);
  bus_.AddDevice(WAVE_PATTERN_START_ADDRESS, WAVE_PATTERN_END_ADDRESS, wave_pattern_md_.get());

  dma_ = 0x00;
  dma_md_ = std::make_unique<FixedPointerWithHandlerMemoryDevice<1, u8>>(&dma_, [this](u16 address, u8 previous, u8 value, bool failed) {
    if (value != 0 && dma_ == 0) {
      StartDMA(value);
      return value;
    }
    return previous;
  }, kMemoryAccessBoth);
  bus.AddDevice(DMA_ADDRESS, dma_md_.get());

  cpu_mode_lock_ = 0;
  cpu_mode_lock_md_ = std::make_unique<FixedPointerWithHandlerMemoryDevice<1, CPUMode>>(&cpu_mode_, [this](u16 address, CPUMode previous, CPUMode value, bool failed) {
    if (value != 0 && previous == 0) {
      cpu_mode_lock_md_->DisableAccess(kMemoryAccessBoth);
      cpu_mode_md_->DisableAccess(kMemoryAccessWrite);
      std::cout << "cpu mode locked" << std::endl;
      return value;
    }
    return previous;
  }, kMemoryAccessBoth);
  cpu_mode_ = kCPUModeDMG;
  cpu_mode_md_ = std::make_unique<FixedPointerWithHandlerMemoryDevice<1, CPUMode>>(&cpu_mode_, [this](u16 address, CPUMode previous, CPUMode value, bool failed) {
    if (value == previous || cpu_mode_lock_ != 0) {
      return previous;
    }
    cpu_mode_lock_ = 0xFF;
    CPUModeChangeEvent event{value};
    event_bus_.Emit(event);
    // todo
    std::cout << "cpu mode changed to " << value << std::endl;
    return value;
  }, kMemoryAccessBoth);

  vram_0_.fill(00);
  vram_md_ = std::make_unique<SwitchingArrayMemoryDevice<VRAM_SIZE>>(VRAM_START_ADDRESS, &vram_0_, kMemoryAccessBoth);
  bus_.AddDevice(VRAM_START_ADDRESS, VRAM_END_ADDRESS, vram_md_.get());

  vram_select_ = 0;
  vram_select_md_ = std::make_unique<FixedPointerWithHandlerMemoryDevice<1, u8>>(&vram_select_, [this](u16 address, u8 previous, u8 value, bool failed){
    if (previous == value) {
      return value;
    }
    value = value & 0x02;
    std::cout << "vram select: " << ToHex(previous) << " -> " << ToHex(value) << std::endl;
    if (value == 0) {
      vram_md_->Switch(&vram_0_);
    } else if (value == 1) {
      vram_md_->Switch(&vram_1_);
    } else {
      value = previous;
    }
    EMIT_BANK_CHANGE(bus_);
    return value;
  }, kMemoryAccessBoth);
  bus_.AddDevice(VRAM_BANK_SELECT_ADDRESS, vram_select_md_.get());

  lcdc_md_ = std::make_unique<FixedPointerWithHandlerMemoryDevice<1, LCDC>>(&lcdc_, [this](u16 address, LCDC previous, LCDC value, bool failed) {
    LCDControlChangeEvent event{value, previous};
    event_bus_.Emit(event);
    return value;
  }, kMemoryAccessBoth);
  bus_.AddDevice(LCD_CONTROL_ADDRESS, lcdc_md_.get());

  lcds_md_ = std::make_unique<FixedPointerWithHandlerMemoryDevice<1, LCDS>>(&lcds_, [this](u16 address, LCDS previous, LCDS value, bool failed) {
    // first two values are read only
    value.bits.ppu_mode = previous.bits.ppu_mode;
    value.bits.lyc_ly_compare = previous.bits.lyc_ly_compare;
    //LCDControlChangeEvent event{value, previous};
    //event_bus_.Emit(event);
    return value;
  }, kMemoryAccessBoth);
  bus_.AddDevice(LCD_STAT_ADDRESS, lcds_md_.get());

  bgp_md_ = std::make_unique<FixedPointerMemoryDevice<1, u8>>(&bgp_, kMemoryAccessBoth);
  bus_.AddDevice(LCD_BGP_ADDRESS, bgp_md_.get());

  bcps_ = 0;
  bgpi_ = 0;
  bcps_bgpi_md_ = std::make_unique<SwitchingPointerMemoryDevice<1, u8>>(&bcps_, kMemoryAccessBoth);
  bus_.AddDevice(LCD_BCPS_BGPI_ADDRESS, bcps_bgpi_md_.get());

  ocps_ = 0;
  obpi_ = 0;
  ocps_obpi_md_ = std::make_unique<SwitchingPointerMemoryDevice<1, u8>>(&ocps_, kMemoryAccessBoth);
  bus_.AddDevice(LCD_OCPS_OBPI_ADDRESS, ocps_obpi_md_.get());

  ocpd_ = 0;
  obpd_ = 0;
  ocpd_obpd_md_ = std::make_unique<SwitchingPointerMemoryDevice<1, u8>>(&ocpd_, kMemoryAccessBoth);
  bus_.AddDevice(LCD_OCPD_OBPD_ADDRESS, ocpd_obpd_md_.get());

  joyp_ = 0;
  joyp_md_ = std::make_unique<FixedPointerMemoryDevice<1, u8>>(&joyp_, kMemoryAccessBoth);
  bus_.AddDevice(JOYP_ADDRESS, joyp_md_.get());

  scx_ = 0;
  scx_md_ = std::make_unique<FixedPointerMemoryDevice<1, u8>>(&scx_, kMemoryAccessBoth);
  bus_.AddDevice(LCD_SCX_ADDRESS, scx_md_.get());

  scy_ = 0;
  scy_md_ = std::make_unique<FixedPointerMemoryDevice<1, u8>>(&scy_, kMemoryAccessBoth);
  bus_.AddDevice(LCD_SCY_ADDRESS, scy_md_.get());

  wx_ = 0;
  wx_md_ = std::make_unique<FixedPointerMemoryDevice<1, u8>>(&wx_, kMemoryAccessBoth);
  bus_.AddDevice(LCD_WX_ADDRESS, wx_md_.get());

  wy_ = 0;
  wy_md_ = std::make_unique<FixedPointerMemoryDevice<1, u8>>(&wy_, kMemoryAccessBoth);
  bus_.AddDevice(LCD_WY_ADDRESS, wy_md_.get());

  ly_ = 0;
  ly_md_ = std::make_unique<FixedPointerMemoryDevice<1, u8>>(&ly_, kMemoryAccessRead);
  bus_.AddDevice(LCD_LY_ADDRESS, ly_md_.get());

  lyc_ = 0;
  lyc_md_ = std::make_unique<FixedPointerMemoryDevice<1, u8>>(&lyc_, kMemoryAccessBoth);
  bus_.AddDevice(LCD_LYC_ADDRESS, lyc_md_.get());

  obp0_ = 0;
  obp0_md_ = std::make_unique<FixedPointerMemoryDevice<1, u8>>(&obp0_, kMemoryAccessBoth);
  bus_.AddDevice(LCD_OBP0_ADDRESS, obp0_md_.get());

  obp1_ = 0;
  obp1_md_ = std::make_unique<FixedPointerMemoryDevice<1, u8>>(&obp1_, kMemoryAccessBoth);
  bus_.AddDevice(LCD_OBP1_ADDRESS, obp1_md_.get());

  sb_ = 0;
  sb_md_ = std::make_unique<FixedPointerMemoryDevice<1, u8>>(&sb_, kMemoryAccessBoth);
  bus_.AddDevice(SB_ADDRESS, sb_md_.get());

  sc_ = 0;
  sc_md_ = std::make_unique<FixedPointerMemoryDevice<1, u8>>(&sc_, kMemoryAccessBoth);
  bus_.AddDevice(SC_ADDRESS, sc_md_.get());

  key1_ = 0;
  key1_md_ = std::make_unique<FixedPointerMemoryDevice<1, u8>>(&key1_, kMemoryAccessBoth);
  bus_.AddDevice(KEY1_ADDRESS, key1_md_.get());
}

void CPU::Stop() {
  /*if (key1_ == 0x01) {
    clock_speed_ = DOUBLE_CPU_CLOCK_SPEED;
    return;
  }
  running_ = false;*/
  halted_ = true;
  // stop is much more complicated than this
  // https://gbdev.io/pandocs/Reducing_Power_Consumption.html#the-bizarre-case-of-the-game-boy-stop-instruction-before-even-considering-timing
}

void CPU::Halt() {
  halted_ = true;
  //std::cout << "halted" << std::endl;
}

void CPU::Step() {
  cycles_consumed_ = 0;
//  std::cout << ToHex(registers_.sp) << std::endl;
  auto instruction = Fetch(alu, registers_, bus_);
#ifdef ENABLE_DEBUGGER
  if (!instruction) {
    DEBUGGER_PAUSE_HERE();
    return;
  }
#else
  if (!instruction) {
    std::cerr << "hit an invalid instruction" << std::endl;
    abort();
  }
#endif
  EMIT_PRE_EXEC_INSTRUCTION();
  int cycles = instruction->Execute(*this, alu, registers_, bus_);
  ic_++;
  EMIT_POST_EXEC_INSTRUCTION();
  cycles_consumed_ += cycles;
}

void CPU::HandleInterrupts() {
  if (ime_) {
    u8 max = static_cast<u8>(kInterruptMax);
    for (u8 i = 0; i < max; ++i) {
      auto type = (InterruptType) (1 << i);
      if ((ie_ & if_ & type) == 0) {
        continue;
      }
      SetInterruptMasterEnable(false, true);
      ClearInterrupt(type);

      u16 address = 0x0040 + (i * 8);
      // cpu waits wait 2 M-cycles
      cycles_consumed_ += 4 * 2; // 8 T-cycles

      // pushing the PC to stack consumes 2 M-cycles
      EMIT_CALL(registers_.pc, registers_.sp, address, true);
      Push(registers_.pc);
      cycles_consumed_ += 4 * 2; // 8 T-cycles

      // changing the PC consumes 1 last M-cycle
      registers_.pc = address;
      cycles_consumed_ += 4 * 1; // 4 T-cycles

      //std::cout << "jump to interrupt: " << ToHex(registers_.pc) << std::endl;
      break;
    }
  }
  if (ime_pending_ != ime_) {
    SetInterruptMasterEnable(ime_pending_, true);
  }
}

void CPU::UpdateTimers(u32 cycles) {
  tic_ += cycles;
  if (tima_ == 0 && tima_overflow_) {
    SendInterrupt(kInterruptTypeTimer);
    tima_ = tma_;
    tic_ = 0;
  }
  tima_overflow_ = false;

  UpdateDivider(cycles);
  if (IsTimerEnabled()) {
    timer_clock_ -= cycles;
    if (timer_clock_ <= 0) {
      u16 left = -timer_clock_;
      SetClockFrequency();
      timer_clock_ -= left;

      if (tima_ == 0xFF) {
        tima_overflow_ = true;
        tima_ = 0;
      } else {
        tima_++;
      }
    }
  }
}

void CPU::SetClockFrequency() {
  u8 freq = tac_ & 0b11;
  switch (freq) {
    case 0: timer_clock_ = 1024; break; // freq 4096
    case 1: timer_clock_ = 16; break;// freq 262144
    case 2: timer_clock_ = 64; break;// freq 65536
    case 3: timer_clock_ = 256; break;// freq 16382
  }
}

void CPU::UpdateDivider(u32 cycles) {
  div_clock_ += cycles;
  if (div_clock_ >= 0xFF) {
    div_clock_ = 0;
    div_++;
  }
}

bool CPU::IsTimerEnabled() {
  return (tac_ & 0b0100) != 0;
}

void CPU::EnableInterrupt(InterruptType type) {
  std::cout << "enable interrupt: " << InterruptTypeToString(type) << std::endl;
  ie_ |= (u8)type;
}

void CPU::DisableInterrupt(InterruptType type) {
  std::cout << "disable interrupt: " << InterruptTypeToString(type) << std::endl;
  ie_ &= ~(u8)type;
}

void CPU::SendInterrupt(InterruptType type) {
  //std::cout << "send interrupt: " << InterruptTypeToString(type) << std::endl;
  if_ |= (u8)type;
  halted_ = false;
}

void CPU::ClearInterrupt(InterruptType type) {
  //std::cout << "clear interrupt: " << InterruptTypeToString(type) << std::endl;
  if_ &= ~(u8)type;
}

void CPU::SetInterruptMasterEnable(bool enable, bool immediate) {
  if (immediate) {
    ime_ = enable;
    ime_pending_ = ime_;
    //std::cout << "interrupt master enable: " << BoolToStr(enable) << std::endl;
  } else {
    ime_pending_ = enable;
    //std::cout << "interrupt master enable pending: " << BoolToStr(enable) << std::endl;
  }
}

void CPU::SetClockSpeed(u32 clock_speed) {
  clock_speed_ = clock_speed;
}

void CPU::Push(u16 value) {
  registers_.sp = registers_.sp - 2;
#ifdef ENABLE_DEBUGGER
  if (!bus_.CheckAccess(registers_.sp, kMemoryAccessBoth) ||
      !bus_.CheckAccess(registers_.sp + 1, kMemoryAccessBoth)) {
    std::cerr << "push stack overflow!" << std::endl;
    DEBUGGER_PAUSE_HERE();
    return;
  }
#endif
  bus_.WriteWord(registers_.sp, value);
  //std::cout << "push: " << ToHex(registers_.sp) << " <- " << ToHex(value) << std::endl;
}

u16 CPU::Pop() {
#ifdef ENABLE_DEBUGGER
  if (!bus_.CheckAccess(registers_.sp, kMemoryAccessBoth) ||
      !bus_.CheckAccess(registers_.sp + 1, kMemoryAccessBoth)) {
    std::cerr << "pop stack overflow!" << std::endl;
    DEBUGGER_PAUSE_HERE();
    return 0xFF;
  }
#endif
  u16 value = bus_.ReadWord(registers_.sp);
  registers_.sp = registers_.sp + 2;
  //std::cout << "pop: " << ToHex(registers_.sp) << " -> " << ToHex(value) << std::endl;
  return value;
}

void CPU::StartDMA(u8 value) {
  //std::cout << "started dma" << std::endl;
  dma_ = value;
  dma_current_ = 0;
  //oam_md_->DisableAccess(kMemoryAccessBoth);
}

void CPU::ProcessDMA() {
  if (dma_ == 0) {
    return;
  }
  int runs = cycles_consumed_ / 4;
  while (runs > 0 && dma_current_ < 0xA0) {
    oam_[dma_current_] = bus_.Read((dma_ * 0x0100) + dma_current_);
    dma_current_++;
    runs--;
  }
  if (dma_current_ >= 0xA0) {
    //std::cout << "finished dma" << std::endl;
    dma_ = 0;
    dma_current_ = 0;
    //oam_md_->EnableAccess(kMemoryAccessBoth);
  }
}

void CPU::OnEvent(Event& event) {

}

void CPU::LoadBootRom(std::vector<u8> data) {
  if (!boot_unloaded_) {
    return;
  }
  boot_unloaded_ = false;
  int size = data.size(); // keep the size before moving
  rom_device_ = std::make_unique<BootROMDevice>(std::move(data));
  rom_size_ = size;
  bus_.AddDevice(0x0000, 0x0100, rom_device_.get());
  if (size > 0x0100) { // for loading DMG rom, probably should get rid of it
    bus_.AddDevice(0x0200, 0x08FF, rom_device_.get());
  }
}

void CPU::UnloadBootRom() {
  if (boot_unloaded_) {
    return;
  }
  boot_unloaded_ = true;
  bus_.PopFrontDevice(0x0000, 0x0100);
  if (rom_size_ > 0x0100) {
    bus_.PopFrontDevice(0x0200, 0x08FF);
  }
  rom_size_ = 0;
  EMIT_ROM_UNMAP(bus_);
}

void CPU::LoadCartridge(std::unique_ptr<Cartridge> cartridge) {
  cartridge_ = std::move(cartridge);
  cartridge_->InitBus(bus_);
}
