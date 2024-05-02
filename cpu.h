#pragma once

#include "register.h"
#include "memory.h"

struct CPU {
  Registers registers_;
  MemoryBus bus_;

  void Step();

  u8 Fetch8();
  u16 Fetch16();

  u8 Add8(u8 first, u8 second);
  u16 Add16(u16 first, u16 second);

  u8 Inc8(u8 first);
  u16 Inc16(u16 first);

  u8 Sub8(u8 first, u8 second);

  u8 Dec8(u8 first);
  u16 Dec16(u16 first);


};