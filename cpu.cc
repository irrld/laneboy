#include "cpu.h"

void CPU::Step() {

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

u8 CPU::Add8(u8 first, u8 second) {
  auto result = add_overflow(first, second);
  registers_.flags.f.carry = result.did_overflow;
  registers_.flags.f.zero = result.value == 0;
  registers_.flags.f.subtract = false;
  registers_.flags.f.half_carry = (first & 0xF) + (second & 0xF) > 0xF;
  return result.value;
}

u16 CPU::Add16(u16 first, u16 second) {
  auto result = add_overflow(first, second);
  return result.value;
}

u8 CPU::Inc8(u8 first) {
  auto result = add_overflow(first, (u8) 1);
  registers_.flags.f.zero = result.value == 0;
  registers_.flags.f.subtract = false;
  registers_.flags.f.half_carry = ((first & 0xF) + 1) > 0xF;
  return result.value;
}

u16 CPU::Inc16(u16 first) {
  return first + 1;
}

u8 CPU::Sub8(u8 first, u8 second) {
  auto result = add_overflow(first, (u8) 1);
  registers_.flags.f.carry = result.did_overflow;
  registers_.flags.f.zero = result.value == 0;
  registers_.flags.f.subtract = true;
  registers_.flags.f.half_carry = ((first & 0xF) + (second & 0xF)) > 0xF;
  return result.value;
}

u8 CPU::Dec8(u8 first) {
  auto result = add_overflow(first, (u8) 1);
  registers_.flags.f.zero = result.value == 0;
  registers_.flags.f.subtract = true;
  registers_.flags.f.half_carry = (first & 0x0F) == 0;
  return result.value;
}

u16 CPU::Dec16(u16 first) {
  return first - 1;
}