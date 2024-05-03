#include "cpu.h"
#include "instructions.h"
#include "debug.h"

CPU::CPU() {
  bus_.Write8(INTERRUPT_ENABLE_ADDRESS, 0);
  bus_.Write8(INTERRUPT_FLAG_ADDRESS, 0);
  bus_.Write8(TIMA_ADDRESS, 0);
  bus_.Write8(TMA_ADDRESS, 0);
  bus_.Write8(DIV_ADDRESS, 0);
  bus_.Write8(TAC_ADDRESS
              , 0);
}
void CPU::Stop() {
  running_ = false;
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
  u8 ine = bus_.Read8(INTERRUPT_ENABLE_ADDRESS);
  u8 inf = bus_.Read8(INTERRUPT_FLAG_ADDRESS);
  for (u8 i = 0; i < max; ++i) {
    if ((ine & inf & (1 << i)) == 0) {
      continue;
    }
    Push(registers_.pc);
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

      u8 tima = bus_.Read8(TIMA_ADDRESS);
      if (((u16)tima + 1) > 0xFF) {
        u8 tma = bus_.Read8(TMA_ADDRESS);
        tima = tma;

        SendInterrupt(InterruptType::Timer);
      } else {
        tima++;
      }
      bus_.Write8(TIMA_ADDRESS, tima);
    }
  }
}

void CPU::UpdateDividerRegister(int cycles) {
  if (div_period_ <= 0) {
    div_period_ = 256;

    u8 div = bus_.Read8(DIV_ADDRESS);
    div++;
    bus_.Write8(DIV_ADDRESS, div);
  }

}
void CPU::SetClockFrequency() {
  u8 tac = bus_.Read8(TAC_ADDRESS);
  switch (tac & 0b11) {
    case 0: timer_period_ = 1024; break; // 4096Hz
    case 1: timer_period_ = 16;   break; // 262144Hz
    case 2: timer_period_ = 64;   break; // 65536Hz
    case 3: timer_period_ = 256;  break; // 16384Hz
  }
}

bool CPU::IsTimerEnabled() {
  u8 tac = bus_.Read8(TAC_ADDRESS);
  return (tac & 0b0100) != 0;
}

void CPU::EnableInterrupt(InterruptType type) {
  std::cout << "enable interrupt: " << InterruptTypeToString(type) << std::endl;
  u8 ine = bus_.Read8(INTERRUPT_ENABLE_ADDRESS);
  ine |= (u8)type;
  bus_.Write8(INTERRUPT_ENABLE_ADDRESS, ine);
}

void CPU::DisableInterrupt(InterruptType type) {
  std::cout << "disable interrupt: " << InterruptTypeToString(type) << std::endl;
  u8 ine = bus_.Read8(INTERRUPT_ENABLE_ADDRESS);
  ine &= ~(u8)type;
  bus_.Write8(INTERRUPT_ENABLE_ADDRESS, ine);
}

void CPU::SendInterrupt(InterruptType type) {
  std::cout << "send interrupt: " << InterruptTypeToString(type) << std::endl;
  u8 inf = bus_.Read8(INTERRUPT_FLAG_ADDRESS);
  inf |= (u8)type;
  bus_.Write8(INTERRUPT_FLAG_ADDRESS, inf);
  halted_ = false;
}

void CPU::ClearInterrupt(InterruptType type) {
  std::cout << "clear interrupt: " << InterruptTypeToString(type) << std::endl;
  u8 inf = bus_.Read8(INTERRUPT_FLAG_ADDRESS);
  inf &= ~(u8)type;
  bus_.Write8(INTERRUPT_FLAG_ADDRESS, inf);
  halted_ = false;
}

void CPU::SetInterruptMasterEnable(bool enable) {
  ime_ = enable;
  std::cout << "interrupt master enable: " << BoolToStr(enable) << std::endl;
}

void CPU::Push(u16 value) {
  registers_.sp -= 2;
  bus_.Write16(registers_.sp, value);
}

u16 CPU::Pop() {
  u16 value = bus_.Read16(registers_.sp);
  registers_.sp += 2;
  return value;
}

u8 CPU::Fetch8() {
  u16 v = bus_.Read8(registers_.pc);
  registers_.pc++;
  return v;
}

u16 CPU::Fetch16() {
  u16 v = bus_.Read16(registers_.pc);
  registers_.pc += 2;
  return v;
}

void CPU::LoadBootRom(const u8* bin, u16 size) {
  u16 first_size = std::min(size, (u16) 0xFF);
  memcpy(bus_.boot_mem_, bin, first_size);
  memset(bus_.boot_map_, true, first_size);
  if (size > 0xFF) {
    u16 second_size = size - 0x0200;
    memset(bus_.boot_map_ + 0x0200, true, second_size);
    memcpy(bus_.boot_mem_ + 0x0200, bin + 0x0200, second_size);
  }
}

void CPU::LoadRom(const u8* bin, u16 size) {
    memcpy(bus_.mem_, bin, size);
    memset(bus_.init_map_, true, size);
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