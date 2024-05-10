#include "alu.h"

ALU::ALU(Registers& registers) : registers_(registers) {}

u8 ALU::Add(u8 first, u8 second) {
  u16 temp_result = (u16)first + (u16)second;
  u8 result = temp_result & 0xFF;
  registers_.flags.f.carry = temp_result > 0xFF;
  registers_.flags.f.zero = result == 0;
  registers_.flags.f.subtract = false;
  registers_.flags.f.half_carry = (((first & 0xF) + (second & 0xF)) & 0x10) == 0x10;
  //registers_.flags.f.half_carry = (first & 0xF) + (second & 0xF) > 0xF;
  return result;
}

u8 ALU::AddWithCarry(u8 first, u8 second) {
  bool carry = registers_.flags.f.carry;
  u16 temp_result = (u16)first + (u16)second + carry;
  u8 result = temp_result & 0xFF;
  registers_.flags.f.carry = temp_result > 0xFF;
  registers_.flags.f.zero = result == 0;
  registers_.flags.f.subtract = false;
  registers_.flags.f.half_carry = (((first & 0xF) + (second & 0xF) + carry) & 0x10) == 0x10;
  //registers_.flags.f.half_carry = (first & 0xF) + (second & 0xF) + carry > 0xF;
  return (u8)(result & 0xFF);
}

u8 ALU::Inc(u8 first) {
  u8 result = ((u16)first + 1) & 0xFF;
  registers_.flags.f.zero = result == 0;
  registers_.flags.f.subtract = false;
  //registers_.flags.f.half_carry = (first & 0xF) == 0xF;
  registers_.flags.f.half_carry = ((first & 0xF) + 1) > 0xF;
  return result;
}

u16 ALU::IncWord(u16 first) {
  return first + 1;
}

u8 ALU::Sub(u8 first, u8 second) {
  u8 result = first - second;
  registers_.flags.f.carry = first < second;
  registers_.flags.f.zero = result == 0;
  registers_.flags.f.subtract = true;
  registers_.flags.f.half_carry = (first & 0xF) < (second & 0xF);
  return result;
}

u8 ALU::SubWithCarry(u8 first, u8 second) {
  bool carry = registers_.flags.f.carry;
  u8 result = first - second - carry;
  registers_.flags.f.zero = result == 0;
  registers_.flags.f.subtract = true;
  registers_.flags.f.half_carry = ((s32)first & 0xF) - (second & 0xF) - carry < 0;
  registers_.flags.f.carry = first < second + carry;
  return result;
}

u8 ALU::Dec(u8 first) {
  u8 result = ((u16)first - 1) & 0xFF;
  registers_.flags.f.zero = result == 0;
  registers_.flags.f.subtract = true;
  registers_.flags.f.half_carry = ((first & 0xF) - 1) < 0x00;
  return result;
}

u16 ALU::DecWord(u16 first) {
  return first - 1;
}