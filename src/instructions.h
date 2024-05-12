#pragma once

#include "cpu.h"
#include "memory.h"
#include "register.h"
#include "debug.h"

enum class InstructionType {
  NOP,
  ADD,
  ADC,
  SUB,
  SBC,
  XOR,
  OR,
  RET,
  RETI,
  JP,
  JR,
  CALL,
  RST,
  RLA,
  RRA,
  LD,
  LDH,
  CP,

  POP,
  PUSH,
  DEC,
  INC,
  CPL,
  CCF,
  SCF,
  AND,

  RLCA,
  RRCA,

  HALT,
  STOP,
  DAA,
  DI,
  EI,

  // extended instruction set
  RLC,
  RRC,
  RL,
  RR,
  SLA,
  SRA,
  SWAP,
  SRL,

  BIT,
  RES,
  SET
};

struct Instruction {
  InstructionType type_;

  Instruction(InstructionType type) : type_(type) {}
  virtual ~Instruction() = default;

  virtual int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) = 0;
};

struct InstructionNoOp : Instruction {
  InstructionNoOp() : Instruction(InstructionType::NOP) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    return 4;
  }
};

enum class LoadOperandType {
  REGISTER,        // R
  AS_ADDRESS,      // [R]
  AS_ADDRESS_INC,  // [R+]
  AS_ADDRESS_DEC   // [R-]
};

struct InstructionLoad : Instruction {
  ArithmeticTarget to_;
  LoadOperandType to_type_;
  ArithmeticTarget from_;
  LoadOperandType from_type_;

  int cycles_;

  InstructionLoad(ArithmeticTarget to, LoadOperandType to_type, ArithmeticTarget from, int cycles)
      : Instruction(InstructionType::LD), to_(to), to_type_(to_type), from_(from), from_type_(LoadOperandType::REGISTER), cycles_(cycles) {}

  InstructionLoad(ArithmeticTarget to, LoadOperandType to_type, ArithmeticTarget from, LoadOperandType from_type, int cycles)
      : Instruction(InstructionType::LD), to_(to), to_type_(to_type), from_(from), from_type_(from_type), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    u16 rr = registers.Get(from_);
    u16 value;
    bool is_address;
    if (from_type_ == LoadOperandType::AS_ADDRESS || from_type_ == LoadOperandType::AS_ADDRESS_INC || from_type_ == LoadOperandType::AS_ADDRESS_DEC) {
      value = bus.Read(rr);
      is_address = true;
    } else {
      value = rr;
      is_address = false;
    }
    if (from_type_ == LoadOperandType::AS_ADDRESS_INC) {
      registers.Set(from_, rr + 1);
    } else if (from_type_ == LoadOperandType::AS_ADDRESS_DEC) {
      registers.Set(from_, rr - 1);
    }

    if (to_type_ == LoadOperandType::AS_ADDRESS || to_type_ == LoadOperandType::AS_ADDRESS_INC || to_type_ == LoadOperandType::AS_ADDRESS_DEC) {
      u16 address = registers.Get(to_);
      if (is_16bit_register(from_) && !is_address) {
        bus.WriteWord(address, value);
      } else {
        assert(value <= 0xFF);
        bus.Write(address, (u8)value);
      }
    } else {
      registers.Set(to_, value);
    }
    if (to_type_ == LoadOperandType::AS_ADDRESS_INC) {
      u16 r2 = registers.Get(to_);
      registers.Set(to_, r2 + 1);
    } else if (to_type_ == LoadOperandType::AS_ADDRESS_DEC) {
      u16 r2 = registers.Get(to_);
      registers.Set(to_, r2 - 1);
    }
    return cycles_;
  }
};

struct InstructionLoadRegisterToRegister : Instruction {
  ArithmeticTarget to_;
  ArithmeticTarget from_;

  int cycles_;

  InstructionLoadRegisterToRegister(ArithmeticTarget to, ArithmeticTarget from, int cycles)
      : Instruction(InstructionType::LD), to_(to), from_(from), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    registers.Set(to_, registers.Get(from_));
    return cycles_;
  }
};

struct InstructionLoadHLSPImmediate : Instruction {
  s8 value_;

  InstructionLoadHLSPImmediate(s8 value)
      : Instruction(InstructionType::LD), value_(value) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    u16 sp = registers.Get(ArithmeticTarget::SP);
    u16 result = sp + value_;
    registers.flags.v = 0;
    registers.flags.f.half_carry = ((sp ^ value_ ^ result) & 0x10) == 0x10;
    registers.flags.f.carry = ((sp ^ value_ ^ result) & 0x100) == 0x100;
    registers.Set(ArithmeticTarget::HL, result);
    return 12;
  }
};

struct InstructionLDH1 : Instruction {
  u8 offset_;
  ArithmeticTarget from_;

  InstructionLDH1(u8 offset, ArithmeticTarget from)
      : Instruction(InstructionType::LDH), offset_(offset), from_(from) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    assert(!is_16bit_register(from_));
    bus.Write(0xFF00 + offset_, (u8)registers.Get(from_));
    return 12;
  }
};

struct InstructionLDH2 : Instruction {
  ArithmeticTarget to_;
  u8 offset_;

  InstructionLDH2(ArithmeticTarget to, u8 offset)
      : Instruction(InstructionType::LDH), offset_(offset), to_(to) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    assert(!is_16bit_register(to_));
    registers.Set(to_, bus.Read(0xFF00 + offset_));
    return 12;
  }
};

struct InstructionLDH3 : Instruction {
  u8 offset_;

  InstructionLDH3(u8 offset)
      : Instruction(InstructionType::LD), offset_(offset) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    bus.Write(0xFF00 + offset_, (u8)registers.Get(ArithmeticTarget::A));
    return 8;
  }
};

struct InstructionLDH4 : Instruction {
  u8 offset_;

  InstructionLDH4(u8 offset)
      : Instruction(InstructionType::LD), offset_(offset) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    registers.Set(ArithmeticTarget::A, bus.Read(0xFF00 + offset_));
    return 12;
  }
};


struct InstructionLoadImmediate : Instruction {
  ArithmeticTarget to_;
  bool to_memory_;
  u16 value_;
  bool is_16bit;

  int cycles_;

  InstructionLoadImmediate(ArithmeticTarget result, bool to_memory, u8 value, int cycles)
      : Instruction(InstructionType::LD), to_(result), to_memory_(to_memory), is_16bit(false), value_(value), cycles_(cycles) {}

  InstructionLoadImmediate(ArithmeticTarget result, bool to_memory, u16 value, int cycles)
      : Instruction(InstructionType::LD), to_(result), to_memory_(to_memory), is_16bit(true), value_(value), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    if (to_memory_) {
      assert(!is_16bit);
      bus.Write(registers.Get(to_), (u8) value_);
    } else {
      registers.Set(to_, value_);
    }
    return cycles_;
  }
};

struct InstructionLoadImmediateAddress : Instruction {
  ArithmeticTarget to_;
  u16 value_;

  int cycles_;

  InstructionLoadImmediateAddress(ArithmeticTarget result, u16 value, int cycles)
      : Instruction(InstructionType::LD), to_(result), value_(value), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    u8 value = bus.Read(value_);
    registers.Set(to_, value);
    return cycles_;
  }
};

struct InstructionLoadToAddress : Instruction {
  u16 value_;
  ArithmeticTarget from_;

  int cycles_;

  InstructionLoadToAddress(u16 value, ArithmeticTarget from, int cycles)
      : Instruction(InstructionType::LD), value_(value), from_(from), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    if (is_16bit_register(from_)) {
      bus.WriteWord(value_, registers.Get(from_));
    } else {
      bus.Write(value_, (u8)registers.Get(from_));
    }
    return cycles_;
  }
};

struct InstructionRotateLeftCircular : Instruction {
  ArithmeticTarget to_;
  bool to_memory_;
  bool set_zero_;

  int cycles_;

  InstructionRotateLeftCircular() : Instruction(InstructionType::RLCA), to_(ArithmeticTarget::A), set_zero_(false), to_memory_(false), cycles_(4) {}

  InstructionRotateLeftCircular(ArithmeticTarget to, int cycles) : Instruction(InstructionType::RLC), to_(to), set_zero_(true), to_memory_(false), cycles_(cycles) {}

  InstructionRotateLeftCircular(ArithmeticTarget to, bool to_memory, int cycles) : Instruction(InstructionType::RLC), to_(to), set_zero_(true), to_memory_(to_memory), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    u16 rr = registers.Get(to_);
    u8 value;
    if (to_memory_) {
      value = bus.Read(rr);
    } else {
      assert(!is_16bit_register(to_));
      value = rr;
    }
    u16 t = (value << 1) + (value >> 7);
    registers.flags.f.carry = t > 0xFF;
    t &= 0xFF;
    registers.flags.f.zero = set_zero_ && t == 0;
    registers.flags.f.half_carry = false;
    registers.flags.f.subtract = false;
    if (to_memory_) {
      bus.Write(rr, (u8)t);
    } else {
      registers.Set(to_, t);
    }
    return cycles_;
  }
};

struct InstructionRotateRightCircular : Instruction {
  ArithmeticTarget to_;
  bool to_memory_;
  bool set_zero_;

  int cycles_;

  InstructionRotateRightCircular() : Instruction(InstructionType::RRCA), to_(ArithmeticTarget::A), set_zero_(false), to_memory_(false), cycles_(4) {}

  InstructionRotateRightCircular(ArithmeticTarget to, int cycles) : Instruction(InstructionType::RRC), to_(to), set_zero_(true), to_memory_(false), cycles_(cycles) {}

  InstructionRotateRightCircular(ArithmeticTarget to, bool to_memory, int cycles) : Instruction(InstructionType::RRC), to_(to), set_zero_(true), to_memory_(to_memory), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    u16 rr = registers.Get(to_);
    u8 value;
    if (to_memory_) {
      value = bus.Read(rr);
    } else {
      assert(!is_16bit_register(to_));
      value = rr;
    }
    u16 t = (value >> 1) + ((value & 1) << 7) + ((value & 1) << 8);
    registers.flags.f.carry = t > 0xFF;
    t &= 0xFF;
    registers.flags.f.zero = set_zero_ && t == 0;
    registers.flags.f.half_carry = false;
    registers.flags.f.subtract = false;
    if (to_memory_) {
      bus.Write(rr, (u8)t);
    } else {
      registers.Set(to_, t);
    }
    return cycles_;
  }
};

struct InstructionRotateLeft : Instruction {
  ArithmeticTarget to_;
  bool to_memory_;
  bool set_zero_;

  int cycles_;

  InstructionRotateLeft() : Instruction(InstructionType::RLA), to_(ArithmeticTarget::A), set_zero_(false), to_memory_(false), cycles_(4) {}

  InstructionRotateLeft(ArithmeticTarget to, int cycles) : Instruction(InstructionType::RL), to_(to), set_zero_(true), to_memory_(false), cycles_(cycles) {}

  InstructionRotateLeft(ArithmeticTarget to, bool to_memory, int cycles) : Instruction(InstructionType::RL), to_(to), set_zero_(true), to_memory_(to_memory), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    u16 rr = registers.Get(to_);
    u8 value;
    if (to_memory_) {
      value = bus.Read(rr);
    } else {
      assert(!is_16bit_register(to_));
      value = rr;
    }
    u16 t = ((value << 1) + registers.flags.f.carry);
    registers.flags.f.carry = t > 0xFF;
    t &= 0xFF;
    registers.flags.f.zero = set_zero_ && t == 0;
    registers.flags.f.half_carry = false;
    registers.flags.f.subtract = false;
    if (to_memory_) {
      bus.Write(rr, (u8)t);
    } else {
      registers.Set(to_, t);
    }
    return cycles_;
  }
};

struct InstructionRotateRight : Instruction {
  ArithmeticTarget to_;
  bool to_memory_;
  bool set_zero_;

  int cycles_;

  InstructionRotateRight() : Instruction(InstructionType::RRA), to_(ArithmeticTarget::A), set_zero_(false), to_memory_(false), cycles_(4) {}

  InstructionRotateRight(ArithmeticTarget to, int cycles) : Instruction(InstructionType::RR), to_(to), set_zero_(true), to_memory_(false), cycles_(cycles) {}

  InstructionRotateRight(ArithmeticTarget to, bool to_memory, int cycles) : Instruction(InstructionType::RR), to_(to), set_zero_(true), to_memory_(to_memory), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    u16 rr = registers.Get(to_);
    u8 value;
    if (to_memory_) {
      value = bus.Read(rr);
    } else {
      assert(!is_16bit_register(to_));
      value = rr;
    }

    u8 t = (value >> 1) + (registers.flags.f.carry << 7);
    registers.flags.f.carry = value & 0x1;
    registers.flags.f.zero = set_zero_ && t == 0;
    registers.flags.f.half_carry = false;
    registers.flags.f.subtract = false;
    if (to_memory_) {
      bus.Write(rr, (u8)t);
    } else {
      registers.Set(to_, t);
    }
    return cycles_;
  }
};

struct InstructionShiftLeftArithmetic : Instruction {
  ArithmeticTarget to_;
  bool to_memory_;

  int cycles_;

  InstructionShiftLeftArithmetic(ArithmeticTarget to, int cycles) : Instruction(InstructionType::SLA), to_(to), to_memory_(false), cycles_(cycles) {}

  InstructionShiftLeftArithmetic(ArithmeticTarget to, bool to_memory, int cycles) : Instruction(InstructionType::SLA), to_(to), to_memory_(to_memory), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    u16 rr = registers.Get(to_);
    u8 value;
    if (to_memory_) {
      value = bus.Read(rr);
    } else {
      assert(!is_16bit_register(to_));
      value = rr;
    }
    registers.flags.f.carry = value >> 7;
    value = value << 1;
    registers.flags.f.zero = value == 0;
    registers.flags.f.half_carry = false;
    registers.flags.f.subtract = false;
    if (to_memory_) {
      bus.Write(rr, value);
    } else {
      registers.Set(to_, value);
    }
    return cycles_;
  }
};

struct InstructionShiftRightArithmetic : Instruction {
  ArithmeticTarget to_;
  bool to_memory_;

  int cycles_;

  InstructionShiftRightArithmetic(ArithmeticTarget to, int cycles) : Instruction(InstructionType::SRA), to_(to), to_memory_(false), cycles_(cycles) {}

  InstructionShiftRightArithmetic(ArithmeticTarget to, bool to_memory, int cycles) : Instruction(InstructionType::SRA), to_(to), to_memory_(to_memory), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    u16 rr = registers.Get(to_);
    u8 value;
    if (to_memory_) {
      value = bus.Read(rr);
    } else {
      assert(!is_16bit_register(to_));
      value = rr;
    }
    registers.flags.f.carry = value & 0x1;
    value = (value >> 1) | (value & (1 << 7));
    registers.flags.f.zero = value == 0;
    registers.flags.f.half_carry = false;
    registers.flags.f.subtract = false;
    if (to_memory_) {
      bus.Write(rr, value);
    } else {
      registers.Set(to_, value);
    }
    return cycles_;
  }
};

struct InstructionShiftRightLogical : Instruction {
  ArithmeticTarget to_;
  bool to_memory_;

  int cycles_;

  InstructionShiftRightLogical(ArithmeticTarget to, int cycles) : Instruction(InstructionType::SRL), to_(to), to_memory_(false), cycles_(cycles) {}

  InstructionShiftRightLogical(ArithmeticTarget to, bool to_memory, int cycles) : Instruction(InstructionType::SRL), to_(to), to_memory_(to_memory), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    u16 rr = registers.Get(to_);
    u8 value;
    if (to_memory_) {
      value = bus.Read(rr);
    } else {
      assert(!is_16bit_register(to_));
      value = rr;
    }
    registers.flags.f.carry = value & 0x1;
    value = value >> 1;
    registers.flags.f.zero = value == 0;
    registers.flags.f.half_carry = false;
    registers.flags.f.subtract = false;
    if (to_memory_) {
      bus.Write(rr, value);
    } else {
      registers.Set(to_, value);
    }
    return cycles_;
  }
};

struct InstructionSwap : Instruction {
  ArithmeticTarget to_;
  bool to_memory_;

  int cycles_;

  InstructionSwap(ArithmeticTarget to, int cycles) : Instruction(InstructionType::SWAP), to_(to), to_memory_(false), cycles_(cycles) {}

  InstructionSwap(ArithmeticTarget to, bool to_memory, int cycles) : Instruction(InstructionType::SWAP), to_(to), to_memory_(to_memory), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    u16 rr = registers.Get(to_);
    u8 value;
    if (to_memory_) {
      value = bus.Read(rr);
    } else {
      assert(!is_16bit_register(to_));
      value = rr;
    }
    u8 upper = value >> 4;
    u8 lower = value & 0x0F;
    value = (lower << 4) | upper;
    registers.flags.f.zero = value == 0;
    registers.flags.f.carry = false;
    registers.flags.f.half_carry = false;
    registers.flags.f.subtract = false;
    if (to_memory_) {
      bus.Write(rr, value);
    } else {
      registers.Set(to_, value);
    }
    return cycles_;
  }
};

struct InstructionBit : Instruction {
  ArithmeticTarget to_;
  bool to_memory_;
  u8 bit_;

  int cycles_;

  InstructionBit(u8 bit, ArithmeticTarget to, int cycles) : Instruction(InstructionType::BIT), bit_(bit), to_(to), to_memory_(false), cycles_(cycles) {}

  InstructionBit(u8 bit, ArithmeticTarget to, bool to_memory, int cycles) : Instruction(InstructionType::BIT), bit_(bit), to_(to), to_memory_(to_memory), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    u8 value;
    if (to_memory_) {
      value = bus.Read(registers.Get(to_));
    } else {
      assert(!is_16bit_register(to_));
      value = registers.Get(to_);
    }
    registers.flags.f.zero = (value & (1 << bit_)) == 0;
    registers.flags.f.half_carry = true;
    registers.flags.f.subtract = false;
    return cycles_;
  }
};

struct InstructionSet : Instruction {
  ArithmeticTarget to_;
  bool to_memory_;
  u8 bit_;
  
  int cycles_;

  InstructionSet(u8 bit, ArithmeticTarget to, int cycles) : Instruction(InstructionType::SET), bit_(bit), to_(to), to_memory_(false), cycles_(cycles) {}

  InstructionSet(u8 bit, ArithmeticTarget to, bool to_memory, int cycles) : Instruction(InstructionType::SET), bit_(bit), to_(to), to_memory_(to_memory), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    u16 rr = registers.Get(to_);
    u8 value;
    if (to_memory_) {
      value = bus.Read(rr);
    } else {
      assert(!is_16bit_register(to_));
      value = rr;
    }
    value |= (1 << bit_);
    if (to_memory_) {
      bus.Write(rr, value);
    } else {
      registers.Set(to_, value);
    }
    return cycles_;
  }
};

struct InstructionRes : Instruction {
  ArithmeticTarget to_;
  bool to_memory_;
  u8 bit_;
  
  int cycles_;
  
  InstructionRes(u8 bit, ArithmeticTarget to, int cycles) : Instruction(InstructionType::RES), bit_(bit), to_(to), to_memory_(false), cycles_(cycles) {}

  InstructionRes(u8 bit, ArithmeticTarget to, bool to_memory, int cycles) : Instruction(InstructionType::RES), bit_(bit), to_(to), to_memory_(to_memory), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    u16 rr = registers.Get(to_);
    u8 value;
    if (to_memory_) {
      value = bus.Read(rr);
    } else {
      assert(!is_16bit_register(to_));
      value = rr;
    }
    value &= ~(1 << bit_);
    if (to_memory_) {
      bus.Write(rr, value);
    } else {
      registers.Set(to_, value);
    }
    return cycles_;
  }
};

struct InstructionStop : Instruction {
  InstructionStop() : Instruction(InstructionType::STOP) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    cpu.Stop();
    return 4;
  }
};

enum class IncDecOperandType { REGISTER, MEMORY };

struct InstructionInc : Instruction {
  ArithmeticTarget to_;
  IncDecOperandType to_type_;
  int cycles_;

  InstructionInc(ArithmeticTarget to, int cycles) : Instruction(InstructionType::INC), to_(to), to_type_(IncDecOperandType::REGISTER), cycles_(cycles) {}

  InstructionInc(ArithmeticTarget to, IncDecOperandType to_type, int cycles) : Instruction(InstructionType::INC), to_(to), to_type_(to_type), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    if (to_type_ == IncDecOperandType::REGISTER) {
      if (is_16bit_register(to_)) {
        u16 result = alu.IncWord(registers.Get(to_));
        registers.Set(to_, result);
      } else {
        u8 result = alu.Inc((u8)registers.Get(to_));
        registers.Set(to_, result);
      }
    } else {
      u16 address = registers.Get(to_);
      u8 value = bus.Read(address);
      u8 result = alu.Inc(value);
      bus.Write(address, result);
    }
    return cycles_;
  }
};

struct InstructionDec : Instruction {
  ArithmeticTarget to_;
  IncDecOperandType to_type_;
  int cycles_;

  InstructionDec(ArithmeticTarget to, int cycles) : Instruction(InstructionType::DEC), to_(to), cycles_(cycles) {}

  InstructionDec(ArithmeticTarget to, IncDecOperandType to_type, int cycles) : Instruction(InstructionType::DEC), to_(to), to_type_(to_type), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    if (to_type_ == IncDecOperandType::REGISTER) {
      if (is_16bit_register(to_)) {
        u16 result = alu.DecWord(registers.Get(to_));
        registers.Set(to_, result);
      } else {
        u8 result = alu.Dec((u8)registers.Get(to_));
        registers.Set(to_, result);
      }
    } else {
      u16 address = registers.Get(to_);
      u8 result = alu.Dec(bus.Read(address));
      bus.Write(address, result);
    }
    return cycles_;
  }
};

struct InstructionAdd : Instruction {
  ArithmeticTarget to_;
  ArithmeticTarget from_;
  bool from_memory_;

  int cycles_;

  InstructionAdd(ArithmeticTarget to, ArithmeticTarget from, bool from_memory, int cycles) : Instruction(InstructionType::ADD), to_(to), from_(from), from_memory_(from_memory), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    u16 value;
    if (from_memory_) {
      value = bus.Read(registers.Get(from_));
    } else {
      value = registers.Get(from_);
    }
    if (is_16bit_register(to_)) {
      u16 first = registers.Get(to_);
      u16 second = value;
      u32 temp_result = (u32)first + (u32)second;
      u16 result = temp_result & 0xFFFF;
      registers.flags.f.subtract = false;
      registers.flags.f.carry = temp_result > 0xFFFF;
      registers.flags.f.half_carry = ((first & 0xfff) + (second & 0xfff)) & 0x1000;
      registers.Set(to_, result);
    } else {
      u8 result = alu.Add(registers.Get(to_), value);
      registers.Set(to_, result);
    }
    return cycles_;
  }
};

struct InstructionAddImmediate : Instruction {
  ArithmeticTarget to_;
  u8 value_;

  int cycles_;

  InstructionAddImmediate(ArithmeticTarget to, u8 value, int cycles) : Instruction(InstructionType::ADD), to_(to), value_(value), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    assert(!is_16bit_register(to_));
    u8 result = alu.Add((u8) registers.Get(to_), value_);
    registers.Set(to_, result);
    return cycles_;
  }
};

struct InstructionAddSPImmediate : Instruction {
  s8 offset_;

  InstructionAddSPImmediate(s8 value) : Instruction(InstructionType::ADD), offset_(value) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    u32 rr = registers.Get(ArithmeticTarget::SP);
    u32 result = rr + offset_;
    registers.flags.f.zero = false;
    registers.flags.f.subtract = false;
    registers.flags.f.half_carry = ((rr & 0x0F) + (offset_ & 0x0F)) > 0x0F;
    registers.flags.f.carry = ((rr & 0xFF) + (offset_ & 0xFF)) > 0xFF;
    registers.Set(ArithmeticTarget::SP, result);
    return 16;
  }
};

struct InstructionRestart : Instruction {
  u16 value_;

  explicit InstructionRestart(u16 value) : Instruction(InstructionType::RST), value_(value) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
#ifdef ENABLE_DEBUGGER
    if (registers.pc - 1 == value_) {
      std::cerr << "paused execution because hit a restart that points to itself." << std::endl;
      DEBUGGER_PAUSE_HERE();
    }
    //std::cout << "restart: " << ToHex(Debugger::GetCurrentInstruction()) << std::endl;
#endif
    cpu.Push(registers.pc);
    registers.pc = value_;
    return 16;
  }
};

struct InstructionAddCarry : Instruction {
  ArithmeticTarget to_;
  ArithmeticTarget from_;
  bool from_memory_;

  int cycles_;

  InstructionAddCarry(ArithmeticTarget to, ArithmeticTarget from, bool from_memory, int cycles) : Instruction(InstructionType::ADC), to_(to), from_(from), from_memory_(from_memory), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    u8 value;
    if (from_memory_) {
      value = bus.Read(registers.Get(from_));
    } else {
      assert(!is_16bit_register(from_));
      value = registers.Get(from_);
    }
    assert(!is_16bit_register(to_));
    u8 result = alu.AddWithCarry((u8) registers.Get(to_), value);
    registers.Set(to_, result);
    return cycles_;
  }
};

struct InstructionAddCarryImmediate : Instruction {
  ArithmeticTarget to_;
  u8 value_;

  int cycles_;

  InstructionAddCarryImmediate(ArithmeticTarget to, u8 value, int cycles) : Instruction(InstructionType::ADC), to_(to), value_(value), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    assert(!is_16bit_register(to_));
    u8 result = alu.AddWithCarry((u8) registers.Get(to_), value_);
    registers.Set(to_, result);
    return cycles_;
  }
};

struct InstructionSub : Instruction {
  ArithmeticTarget to_;
  ArithmeticTarget from_;
  bool from_memory_;

  int cycles_;

  InstructionSub(ArithmeticTarget to, ArithmeticTarget from, bool from_memory, int cycles) : Instruction(InstructionType::SUB), to_(to), from_(from), from_memory_(from_memory), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    assert(!is_16bit_register(to_));
    u8 value;
    if (from_memory_) {
      value = bus.Read(registers.Get(from_));
    } else {
      assert(!is_16bit_register(from_));
      value = registers.Get(from_);
    }
    u8 result = alu.Sub(registers.Get(to_), value);
    registers.Set(to_, result);
    return cycles_;
  }
};

struct InstructionSubImmediate : Instruction {
  ArithmeticTarget to_;
  u8 value_;

  int cycles_;

  InstructionSubImmediate(ArithmeticTarget to, u8 value, int cycles) : Instruction(InstructionType::SUB), to_(to), value_(value), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    assert(!is_16bit_register(to_));
    u8 result = alu.Sub((u8) registers.Get(to_), value_);
    registers.Set(to_, result);
    return cycles_;
  }
};

struct InstructionSubCarry : Instruction {
  ArithmeticTarget to_;
  ArithmeticTarget from_;
  bool from_memory_;

  int cycles_;

  InstructionSubCarry(ArithmeticTarget to, ArithmeticTarget from, bool from_memory, int cycles) : Instruction(InstructionType::SBC), to_(to), from_(from), from_memory_(from_memory), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    u16 value;
    if (from_memory_) {
      value = bus.Read(registers.Get(from_));
    } else {
      value = registers.Get(from_);
    }
    assert(!is_16bit_register(to_));
    u8 result = alu.SubWithCarry((u8) registers.Get(to_), value);
    registers.Set(to_, result);
    return cycles_;
  }
};

struct InstructionSubCarryImmediate : Instruction {
  ArithmeticTarget to_;
  u8 value_;

  int cycles_;

  InstructionSubCarryImmediate(ArithmeticTarget to, u8 value, int cycles) : Instruction(InstructionType::SBC), to_(to), value_(value), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    assert(!is_16bit_register(to_));
    u8 result = alu.SubWithCarry((u8) registers.Get(to_), value_);
    registers.Set(to_, result);
    return cycles_;
  }
};

struct InstructionAnd : Instruction {
  ArithmeticTarget to_;
  ArithmeticTarget from_;
  bool from_memory_;

  int cycles_;

  InstructionAnd(ArithmeticTarget to, ArithmeticTarget from, bool from_memory, int cycles) : Instruction(InstructionType::AND), to_(to), from_(from), from_memory_(from_memory), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    u16 value;
    if (from_memory_) {
      value = bus.Read(registers.Get(from_));
    } else {
      value = registers.Get(from_);
    }
    u16 result = registers.Get(to_) & value;
    registers.Set(to_, result);
    registers.flags.f.zero = result == 0;
    registers.flags.f.subtract = false;
    registers.flags.f.half_carry = true;
    registers.flags.f.carry = false;
    return cycles_;
  }
};

struct InstructionAndImmediate : Instruction {
  ArithmeticTarget to_;
  u8 value_;

  int cycles_;

  InstructionAndImmediate(ArithmeticTarget to, u8 value, int cycles) : Instruction(InstructionType::AND), to_(to), value_(value), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    u16 result = registers.Get(to_) & value_;
    registers.Set(to_, result);
    registers.flags.f.zero = result == 0;
    registers.flags.f.subtract = false;
    registers.flags.f.half_carry = true;
    registers.flags.f.carry = false;
    return cycles_;
  }
};

struct InstructionXOR : Instruction {
  ArithmeticTarget to_;
  ArithmeticTarget from_;
  bool from_memory_;

  int cycles_;

  InstructionXOR(ArithmeticTarget to, ArithmeticTarget from, bool from_memory, int cycles) : Instruction(InstructionType::XOR), to_(to), from_(from), from_memory_(from_memory), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    assert(!is_16bit_register(to_));
    u8 value;
    if (from_memory_) {
      value = bus.Read(registers.Get(from_));
    } else {
      assert(!is_16bit_register(from_));
      value = registers.Get(from_);
    }
    u8 result = registers.Get(to_) ^ value;
    registers.Set(to_, result);
    registers.flags.f.zero = result == 0;
    registers.flags.f.subtract = false;
    registers.flags.f.half_carry = false;
    registers.flags.f.carry = false;
    return cycles_;
  }
};

struct InstructionXORImmediate : Instruction {
  ArithmeticTarget to_;
  u8 value_;

  int cycles_;

  InstructionXORImmediate(ArithmeticTarget to, u8 value, int cycles) : Instruction(InstructionType::XOR), to_(to), value_(value), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    assert(!is_16bit_register(to_));
    u8 result = registers.Get(to_) ^ value_;
    registers.Set(to_, result);
    registers.flags.f.zero = result == 0;
    registers.flags.f.subtract = false;
    registers.flags.f.half_carry = false;
    registers.flags.f.carry = false;
    return cycles_;
  }
};

struct InstructionOr : Instruction {
  ArithmeticTarget to_;
  ArithmeticTarget from_;
  bool from_memory_;

  int cycles_;

  InstructionOr(ArithmeticTarget to, ArithmeticTarget from, bool from_memory, int cycles) : Instruction(InstructionType::OR), to_(to), from_(from), from_memory_(from_memory), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    assert(!is_16bit_register(to_));
    u8 value;
    if (from_memory_) {
      value = bus.Read(registers.Get(from_));
    } else {
      assert(!is_16bit_register(from_));
      value = registers.Get(from_);
    }
    u8 result = registers.Get(to_) | value;
    registers.Set(to_, result);
    registers.flags.f.zero = result == 0;
    registers.flags.f.subtract = false;
    registers.flags.f.half_carry = false;
    registers.flags.f.carry = false;
    return cycles_;
  }
};

struct InstructionOrImmediate : Instruction {
  ArithmeticTarget to_;
  u16 value_;

  int cycles_;

  InstructionOrImmediate(ArithmeticTarget to, u16 value, int cycles) : Instruction(InstructionType::OR), to_(to), value_(value), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    assert(!is_16bit_register(to_));
    u16 result = registers.Get(to_) | value_;
    registers.Set(to_, result);
    registers.flags.f.zero = result == 0;
    registers.flags.f.subtract = false;
    registers.flags.f.half_carry = false;
    registers.flags.f.carry = false;
    return cycles_;
  }
};

struct InstructionCompare : Instruction {
  ArithmeticTarget to_;
  ArithmeticTarget from_;
  bool from_memory_;

  int cycles_;

  InstructionCompare(ArithmeticTarget to, ArithmeticTarget from, bool from_memory, int cycles) : Instruction(InstructionType::CP), to_(to), from_(from), from_memory_(from_memory), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    assert(!is_16bit_register(to_));
    u8 value;
    if (from_memory_) {
      value = bus.Read(registers.Get(from_));
    } else {
      assert(!is_16bit_register(from_));
      value = registers.Get(from_);
    }
    u8 original = registers.Get(to_);
    alu.Sub(original, value);
    return cycles_;
  }
};

struct InstructionCompareImmediate : Instruction {
  ArithmeticTarget to_;
  u8 value_;

  int cycles_;

  InstructionCompareImmediate(ArithmeticTarget to, u8 value, int cycles) : Instruction(InstructionType::CP), to_(to), value_(value), cycles_(cycles) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    assert(!is_16bit_register(to_));
    u8 original = registers.Get(to_);
    registers.flags.f.zero = original == value_;
    registers.flags.f.subtract = true;
    registers.flags.f.half_carry = (original & 0xF) < (value_ & 0xF);
    registers.flags.f.carry = original < value_;
    return cycles_;
  }
};

struct InstructionJumpRelative : Instruction {
  s8 value_;

  InstructionJumpRelative(s8 value) : Instruction(InstructionType::JR), value_(value) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    EMIT_JUMP_RELATIVE(registers.pc, registers.sp, value_);
    registers.pc = (registers.pc + value_) & 0xFFFF;
    return 12;
  }
};

struct InstructionJumpRelativeIfZero : Instruction {
  s8 value_;
  bool is_not_;

  InstructionJumpRelativeIfZero(s8 value, bool is_not) : Instruction(InstructionType::JR), value_(value), is_not_(is_not) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    if (registers.flags.f.zero != is_not_) {
      EMIT_JUMP_RELATIVE(registers.pc, registers.sp, value_);
      registers.pc = (registers.pc + value_) & 0xFFFF;
      return 12;
    }
    return 8;
  }
};

struct InstructionJumpRelativeIfCarry : Instruction {
  s8 value_;
  bool is_not_;

  InstructionJumpRelativeIfCarry(s8 value, bool is_not) : Instruction(InstructionType::JR), value_(value), is_not_(is_not) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    if (registers.flags.f.carry != is_not_) {
      EMIT_JUMP_RELATIVE(registers.pc, registers.sp, value_);
      registers.pc = (registers.pc + value_) & 0xFFFF;
      return 12;
    }
    return 8;
  }
};

struct InstructionJump : Instruction {
  u16 value_;

  InstructionJump(u16 value) : Instruction(InstructionType::JP), value_(value) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    EMIT_JUMP(registers.pc, registers.sp, value_);
    registers.pc = value_;
    return 16;
  }
};

struct InstructionJumpIfZero : Instruction {
  u16 value_;
  bool is_not_;

  InstructionJumpIfZero(u16 value, bool is_not) : Instruction(InstructionType::JP), value_(value), is_not_(is_not) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    if (registers.flags.f.zero != is_not_) {
      EMIT_JUMP(registers.pc, registers.sp, value_);
      registers.pc = value_;
      return 16;
    }
    return 12;
  }
};

struct InstructionJumpIfCarry : Instruction {
  u16 value_;
  bool is_not_;

  InstructionJumpIfCarry(u16 value, bool is_not) : Instruction(InstructionType::JP), value_(value), is_not_(is_not) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    if (registers.flags.f.carry != is_not_) {
      EMIT_JUMP(registers.pc, registers.sp, value_);
      registers.pc = value_;
      return 16;
    }
    return 12;
  }
};

struct InstructionCall : Instruction {
  u16 value_;

  explicit InstructionCall(u16 value) : Instruction(InstructionType::CALL), value_(value) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    EMIT_CALL(registers.pc, registers.sp, value_);
    cpu.Push(registers.pc);
    registers.pc = value_;
    return 24;
  }
};

struct InstructionCallIfZero : Instruction {
  u16 value_;
  bool is_not_;

  InstructionCallIfZero(u16 value, bool is_not) : Instruction(InstructionType::CALL), value_(value), is_not_(is_not) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    if (registers.flags.f.zero != is_not_) {
      EMIT_CALL(registers.pc, registers.sp, value_);
      cpu.Push(registers.pc);
      registers.pc = value_;
      return 24;
    }
    return 12;
  }
};

struct InstructionCallIfCarry : Instruction {
  u16 value_;
  bool is_not_;

  InstructionCallIfCarry(u16 value, bool is_not) : Instruction(InstructionType::CALL), value_(value), is_not_(is_not) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    if (registers.flags.f.carry != is_not_) {
      EMIT_CALL(registers.pc, registers.sp, value_);
      cpu.Push(registers.pc);
      registers.pc = value_;
      return 24;
    }
    return 12;
  }
};

// Decimal Adjust Accumulator
struct InstructionDAA : Instruction {

  InstructionDAA() : Instruction(InstructionType::DAA) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    u16 a = registers.a;
    s8 add = 0;
    if ((!registers.flags.f.subtract && (a & 0xf) > 0x9) || registers.flags.f.half_carry){
      add |= 0x6;
    }
    if ((!registers.flags.f.subtract && a > 0x99) || registers.flags.f.carry) {
      add |= 0x60;
      registers.flags.f.carry = true;
    }
    a += registers.flags.f.subtract ? -add : add;
    registers.flags.f.zero = a == 0;
    registers.flags.f.half_carry = false;
    registers.a = a;
    return 4;
  }
};

// Logical Not
struct InstructionComplement : Instruction {

  InstructionComplement() : Instruction(InstructionType::CPL) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    registers.a = 0xFF ^ registers.a;
    registers.flags.f.subtract = true;
    registers.flags.f.half_carry = true;
    return 4;
  }
};

struct InstructionComplementCarryFlag : Instruction {

  InstructionComplementCarryFlag() : Instruction(InstructionType::CCF) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    registers.flags.f.carry = !registers.flags.f.carry;
    registers.flags.f.subtract = false;
    registers.flags.f.half_carry = false;
    return 4;
  }
};

struct InstructionSetCarryFlag : Instruction {

  InstructionSetCarryFlag() : Instruction(InstructionType::SCF) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    registers.flags.f.carry = true;
    registers.flags.f.subtract = false;
    registers.flags.f.half_carry = false;
    return 4;
  }
};

struct InstructionHalt : Instruction {

  InstructionHalt() : Instruction(InstructionType::HALT) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    cpu.Halt();
    return 4;
  }
};

struct InstructionReturn : Instruction {

  bool from_interrupt_;

  InstructionReturn(bool from_interrupt) : Instruction(from_interrupt ? InstructionType::RETI : InstructionType::RET), from_interrupt_(from_interrupt) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    u16 sp = registers.sp;
    u16 return_address = cpu.Pop();
    EMIT_RET(registers.pc, sp, return_address, from_interrupt_);
    registers.pc = return_address;
    if (from_interrupt_) {
      cpu.SetInterruptMasterEnable(true);
    }
    return 16;
  }
};

struct InstructionReturnIfZero : Instruction {

  bool is_not_;

  InstructionReturnIfZero(bool is_not) : Instruction(InstructionType::RET), is_not_(is_not) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    if (registers.flags.f.zero != is_not_) {
      u16 return_address = cpu.Pop();
      EMIT_RET(registers.pc, registers.sp, return_address, false);
      registers.pc = return_address;
      return 20;
    }
    return 8;
  }
};

struct InstructionReturnIfCarry : Instruction {

  bool is_not_;

  InstructionReturnIfCarry(bool is_not) : Instruction(InstructionType::RET), is_not_(is_not) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    if (registers.flags.f.carry != is_not_) {
      u16 return_address = cpu.Pop();
      EMIT_RET(registers.pc, registers.sp, return_address, false);
      registers.pc = return_address;
      return 20;
    }
    return 8;
  }
};

struct InstructionPop : Instruction {

  ArithmeticTarget to_;

  InstructionPop(ArithmeticTarget to) : Instruction(InstructionType::POP), to_(to) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    registers.Set(to_, cpu.Pop());
    return 12;
  }
};

struct InstructionPopAF : Instruction {

  InstructionPopAF() : Instruction(InstructionType::POP) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    u16 value = cpu.Pop();
    // A        F
    // 00000000 00000000
    registers.a = value >> 8;
    registers.flags.v = value & 0xF0;
    return 12;
  }
};

struct InstructionPush : Instruction {

  ArithmeticTarget to_;

  InstructionPush(ArithmeticTarget to) : Instruction(InstructionType::PUSH), to_(to) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    cpu.Push(registers.Get(to_));
    return 16;
  }
};

struct InstructionPushAF : Instruction {

  InstructionPushAF() : Instruction(InstructionType::PUSH) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    u16 af = ((u16)registers.a << 8) | registers.flags.v;
    cpu.Push(af);
    return 16;
  }
};

struct InstructionDisableInterrupt : Instruction {

  InstructionDisableInterrupt() : Instruction(InstructionType::DI) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    cpu.SetInterruptMasterEnable(false);
    return 4;
  }
};

struct InstructionEnableInterrupt : Instruction {

  InstructionEnableInterrupt() : Instruction(InstructionType::EI) {}

  int Execute(CPU& cpu, ALU& alu, Registers& registers, MemoryBus& bus) override {
    cpu.SetInterruptMasterEnable(true);
    return 4;
  }
};

// Extended ($CB prefixed)
std::unique_ptr<Instruction> FetchPrefixed(ALU& alu, Registers& registers, MemoryBus& bus);

std::unique_ptr<Instruction> Fetch(ALU& alu, Registers& registers, MemoryBus& bus);