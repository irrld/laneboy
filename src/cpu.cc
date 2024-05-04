#include "cpu.h"
#include "instructions.h"
#include "debug.h"

class BootROMDevice : public MemoryDevice {
 public:
  BootROMDevice(const u8* data, u16 size) : data_(data), size_(size) {

  }

  bool CheckAccess(u16 address, AccessType type) override {
      return address < size_ && type == AccessType::Read;
  }

  u8 Read(u16 address) override {
    return data_[address];
  }

  void Write(u16 address, u8 value) override {
  }

 private:
  const u8* data_;
  u16 size_;
};

CPU::CPU(MemoryBus& bus) : bus_(bus) {
  SetClockSpeed(BASE_CPU_CLOCK_SPEED);

  ie_md_ = std::make_unique<SinglePointerMemoryDevice>(&ie_, true, true);
  bus_.AddDevice(INTERRUPT_ENABLE_ADDRESS, ie_md_.get(), true);
  if_md_ = std::make_unique<SinglePointerMemoryDevice>(&if_, true, true);
  bus_.AddDevice(INTERRUPT_FLAG_ADDRESS, if_md_.get(), true);
  div_md_ = std::make_unique<SinglePointerMemoryDevice>(&div_, true, true);
  bus_.AddDevice(DIV_ADDRESS, div_md_.get(), true);
  tima_md_ = std::make_unique<SinglePointerMemoryDevice>(&tima_, true, true);
  bus_.AddDevice(TIMA_ADDRESS, tima_md_.get(), true);
  tma_md_ = std::make_unique<SinglePointerMemoryDevice>(&tma_, true, true);
  bus_.AddDevice(TMA_ADDRESS, tma_md_.get(), true);
  tac_md_ = std::make_unique<SinglePointerMemoryDevice>(&tac_, true, true);
  bus_.AddDevice(TAC_ADDRESS, tac_md_.get(), true);
  boot_unmap_md_ = std::make_unique<CallbackOnWriteMemoryDevice>(&boot_unloaded_, [this](u16 address, u8 previous, u8 value){
    if (value != 0) {
      UnloadBootRom();
    }
  });
  bus_.AddDevice(BOOT_UNMAP_ADDRESS, boot_unmap_md_.get(), true);

  // WRAM 0 is fixed
  wram_0_md_ = std::make_unique<FixedMemoryDevice<WRAM_SIZE>>(WRAM_0_START_ADDRESS, wram_0_, true, true);
  bus_.AddDevice(WRAM_0_START_ADDRESS, WRAM_0_START_ADDRESS + WRAM_SIZE, wram_0_md_.get());

  // WRAM 1 is selectable
  wram_select_ = 0x01;
  wram_1_7_md_ = std::make_unique<SwitchingMemoryDevice<WRAM_SIZE>>(WRAM_1_7_START_ADDRESS, wram_1_, true, true);
  bus_.AddDevice(WRAM_1_7_START_ADDRESS, WRAM_1_7_START_ADDRESS + WRAM_SIZE, wram_1_7_md_.get());
  wram_select_md_ = std::make_unique<CallbackOnWriteMemoryDevice>(&wram_select_, [this](u16 address, u8 previous, u8 value){
    if (previous == value) {
      return;
    }
    std::cout << "wram select: " << ToHex(previous) << " -> " << ToHex(value) << std::endl;
    switch (value) {
      case 0:
      case 1:
        wram_1_7_md_->Switch(wram_1_);
        break;
      case 2:
        wram_1_7_md_->Switch(wram_2_);
        break;
      case 3:
        wram_1_7_md_->Switch(wram_3_);
        break;
      case 4:
        wram_1_7_md_->Switch(wram_4_);
        break;
      case 5:
        wram_1_7_md_->Switch(wram_5_);
        break;
      case 6:
        wram_1_7_md_->Switch(wram_6_);
        break;
      case 7:
        wram_1_7_md_->Switch(wram_7_);
        break;
      default:
        wram_select_ = previous;
        break;
    }
  });
  bus_.AddDevice(WRAM_BANK_SELECT_ADDRESS, wram_select_md_.get());
}

void CPU::Stop() {
  running_ = false;
  // stop is much more complicated than this
  // https://gbdev.io/pandocs/Reducing_Power_Consumption.html#the-bizarre-case-of-the-game-boy-stop-instruction-before-even-considering-timing
}

void CPU::Halt() {
  halted_ = true;
  std::cout << "halted" << std::endl;
}

void CPU::Step() {
//  std::cout << ToHex(registers_.sp) << std::endl;
  auto instruction = Fetch(*this, registers_, bus_);
  if (instruction) {
    EMIT_PRE_EXEC_INSTRUCTION();
    int cycles = instruction->Execute(*this, registers_, bus_);
    EMIT_POST_EXEC_INSTRUCTION();
    UpdateTimers(cycles);
    current_cycle_ += cycles;
    ic_++;
  } else {
    std::cout << "unknown instruction" << std::endl;
    Stop();
  }
}

void CPU::HandleInterrupts() {
  if (!ime_) {
    return;
  }
  u8 max = static_cast<u8>(InterruptType::InterruptMax);
  for (u8 i = 0; i < max; ++i) {
    if ((ie_ & if_ & (1 << i)) == 0) {
      continue;
    }
    // cpu waits wait 2 M-cycles
    current_cycle_ += 4 * 2; // 8 T-cycles
    // todo we don't actually wait here, it would be better to make the cpu execute
    // two nop instructions before doing anything (in CPU::Step)
    Push(registers_.pc);
    // pushing the PC to stack consumes 2 M-cycles
    current_cycle_ += 4 * 2; // 8 T-cycles
    // changing the PC consumes 1 last M-cycle
    current_cycle_ += 4 * 1; // 4 T-cycles
    registers_.pc = 0x0040 + i * 8;
    ClearInterrupt((InterruptType) i);
    break;
  }
}

void CPU::UpdateTimers(int cycles) {
  UpdateDividerRegister(cycles);
  if (IsTimerEnabled()) {
    timer_period_ -= cycles;

    if (timer_period_ <= 0) {
      SetClockFrequency();

      if (((u16)tima_ + 1) > 0xFF) {
        tima_ = tma_;

        SendInterrupt(InterruptType::Timer);
      } else {
        tima_++;
      }
    }
  }
}

void CPU::UpdateDividerRegister(int cycles) {
  if (div_period_ <= 0) {
    div_period_ = 256;
    div_++;
  }
}

void CPU::SetClockFrequency() {
  switch (tac_ & 0b11) {
    case 0: timer_period_ = 1024; break; // 4096Hz
    case 1: timer_period_ = 16;   break; // 262144Hz
    case 2: timer_period_ = 64;   break; // 65536Hz
    case 3: timer_period_ = 256;  break; // 16384Hz
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
  std::cout << "send interrupt: " << InterruptTypeToString(type) << std::endl;
  if_ |= (u8)type;
  halted_ = false;
}

void CPU::ClearInterrupt(InterruptType type) {
  std::cout << "clear interrupt: " << InterruptTypeToString(type) << std::endl;
  if_ &= ~(u8)type;
}

void CPU::SetInterruptMasterEnable(bool enable) {
  ime_ = enable;
  std::cout << "interrupt master enable: " << BoolToStr(enable) << std::endl;
}

void CPU::SetClockSpeed(u32 clock_speed) {
  clock_speed_ = clock_speed;
  timer_period_ = clock_speed / 4096;
  div_period_ = clock_speed / 16384;
}

void CPU::Push(u16 value) {
  registers_.sp -= 2;
  bus_.WriteWord(registers_.sp, value);
}

u16 CPU::Pop() {
  u16 value = bus_.ReadWord(registers_.sp);
  registers_.sp += 2;
  return value;
}

u8 CPU::Fetch8() {
  u16 v = bus_.Read(registers_.pc);
  registers_.pc++;
  return v;
}

u16 CPU::Fetch16() {
  u16 v = bus_.ReadWord(registers_.pc);
  registers_.pc += 2;
  return v;
}

void CPU::LoadBootRom(const u8* bin, u16 size) {
  if (!boot_unloaded_) {
    return;
  }
  boot_unloaded_ = false;
  rom_device_ = std::make_unique<BootROMDevice>(bin, size);
  rom_size_ = size;
  bus_.AddDevice(0x0000, 0x0100, rom_device_.get());
  if (size > 0x0100) {
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
}

void CPU::LoadCartridge(std::unique_ptr<Cartridge> cartridge) {
  cartridge_ = std::move(cartridge);
  cartridge_->InitBus(bus_);
}

u8 CPU::Add(u8 first, u8 second) {
  u16 result = static_cast<u16>(first) + static_cast<u16>(second);
  registers_.flags.f.carry = result > 0xFF;
  result = static_cast<u8>(result);
  registers_.flags.f.zero = result == 0;
  registers_.flags.f.subtract = false;
  registers_.flags.f.half_carry = (first & 0xF) + (second & 0xF) > 0xF;
  return (u8)(result & 0xFF);
}

u8 CPU::AddWithCarry(u8 first, u8 second) {
  bool carry = registers_.flags.f.carry;
  u16 result = static_cast<u16>(first) + static_cast<u16>(second) + carry;
  registers_.flags.f.carry = result > 0xFF;
  result = static_cast<u8>(result);
  registers_.flags.f.zero = result == 0;
  registers_.flags.f.subtract = false;
  registers_.flags.f.half_carry = (first & 0xF) + (second & 0xF) + carry > 0xF;
  return (u8)(result & 0xFF);
}

u16 CPU::AddWord(u16 first, u16 second) {
  return first + second;
}

u8 CPU::Inc(u8 first) {
  u8 result = static_cast<u8>(static_cast<u16>(first) + 1);
  registers_.flags.f.zero = result == 0;
  registers_.flags.f.subtract = false;
  registers_.flags.f.half_carry = ((first & 0xF) + 1) > 0xF;
  return result;
}

u16 CPU::IncWord(u16 first) {
  return first + 1;
}

u8 CPU::Sub(u8 first, u8 second) {
  u16 result = static_cast<u16>(first) - static_cast<u16>(second);
  registers_.flags.f.carry = result > 0xFF;
  result = static_cast<u8>(result);
  registers_.flags.f.zero = result == 0;
  registers_.flags.f.subtract = true;
  registers_.flags.f.half_carry = (first & 0xF) - (second & 0xF) > 0xF;
  return (u8)(result & 0xFF);
}

u16 CPU::SubWord(u16 first, u16 second) {
  return first - second;
}

u8 CPU::SubWithCarry(u8 first, u8 second) {
  bool carry = registers_.flags.f.carry;
  u16 result = static_cast<u16>(first) - static_cast<u16>(second) - carry;
  registers_.flags.f.carry = result > 0xFF;
  result = static_cast<u8>(result);
  registers_.flags.f.zero = result == 0;
  registers_.flags.f.subtract = true;
  registers_.flags.f.half_carry = (first & 0xF) - (second & 0xF) - carry > 0xF;
  return (u8)(result & 0xFF);
}

u8 CPU::Dec(u8 first) {
  u8 result = static_cast<u8>(static_cast<u16>(first) - 1);
  registers_.flags.f.zero = result == 0;
  registers_.flags.f.subtract = true;
  registers_.flags.f.half_carry = ((first & 0xF) - 1) > 0xF;
  return result;
}

u16 CPU::DecWord(u16 first) {
  return first - 1;
}