
#pragma once

#include "memory.h"
#include "register.h"

class ALU {
 public:
  ALU(Registers& registers);

  u8 Add(u8 first, u8 second);
  u8 AddWithCarry(u8 first, u8 second);
  u8 AddWithCarry(u16 first, u8 second) = delete;
  u8 AddWithCarry(u8 first, u16 second) = delete;

  u8 Inc(u8 first);
  u16 IncWord(u16 first);

  u8 Sub(u8 first, u8 second);
  u8 SubWithCarry(u8 first, u8 second);

  u8 Dec(u8 first);
  u16 DecWord(u16 first);

 private:
  Registers& registers_;
};
