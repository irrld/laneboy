#pragma once

#include "register.h"
#include "memory.h"
#include "cpu.h"

enum class InstructionType {
  NOP,
  ADD,
  ADC,
  SUB,
  SBC,
  ADND,
  XOR,
  OR,
  RET,
  RETI,
  JP,
  CALL,
  RST,
  RLA,
  RRA,
  LD,

  POP,
  PUSH,
  DEC,
  INC,

  RLCA,
  RRCA,

  HALT,
  STOP,

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

  virtual int Execute(CPU& cpu, Registers& registers, MemoryBus& bus) = 0;

};

struct InstructionNoOp : Instruction {
  InstructionNoOp() : Instruction(InstructionType::NOP) {

  }

  int Execute(CPU& cpu, Registers& registers, MemoryBus& bus) override {
    return 4;
  }
};

struct InstructionLoad : Instruction {
  ArithmeticTarget to_;
  bool to_memory_;
  ArithmeticTarget from_;
  bool from_memory_;

  int cycles_;

  InstructionLoad(ArithmeticTarget to, bool to_memory, ArithmeticTarget from, bool from_memory, int cycles)
    : Instruction(InstructionType::LD), to_(to), to_memory_(to_memory), from_(from), from_memory_(from_memory), cycles_(cycles) { }

  int Execute(CPU& cpu, Registers &registers, MemoryBus& bus) override {
    u16 value;
    if (from_memory_) {
      value = bus.Read8(registers.Get(from_));
    } else {
      value = registers.Get(from_);
    }
    if (to_memory_) {
      bus.Write8(registers.Get(to_), value);
    } else {
      registers.Set(to_, value);
    }
    return cycles_;
  }
};

struct InstructionLoadImmediate : Instruction {
  ArithmeticTarget to_;
  bool to_memory_;
  u16 value_;
  bool is_16bit;

  int cycles_;

  InstructionLoadImmediate(ArithmeticTarget result, bool to_memory, u8 value, int cycles)
      : Instruction(InstructionType::LD), to_(result), to_memory_(to_memory), is_16bit(false), value_(value), cycles_(cycles) { }

  InstructionLoadImmediate(ArithmeticTarget result, bool to_memory, u16 value, int cycles)
      : Instruction(InstructionType::LD), to_(result), to_memory_(to_memory), is_16bit(true), value_(value), cycles_(cycles) { }

  int Execute(CPU& cpu, Registers &registers, MemoryBus& bus) override {
    if (to_memory_) {
      if (is_16bit) {
        bus.Write16(registers.Get(to_), value_);
      } else {
        bus.Write8(registers.Get(to_), value_);
      }
    } else {
      registers.Set(to_, value_);
    }
    return cycles_;
  }
};


struct InstructionLoadToAddress : Instruction {
  u16 value_;
  ArithmeticTarget from_;
  bool from_memory_;
  bool is_16bit_;

  int cycles_;

  InstructionLoadToAddress(u16 value, ArithmeticTarget from, bool from_memory, int cycles)
      : Instruction(InstructionType::LD), value_(value), is_16bit_(true), from_(from), from_memory_(from_memory), cycles_(cycles) { }

  InstructionLoadToAddress(u8 value, ArithmeticTarget from, bool from_memory, int cycles)
      : Instruction(InstructionType::LD), value_(value), is_16bit_(false), from_(from), from_memory_(from_memory), cycles_(cycles) { }

  int Execute(CPU& cpu, Registers &registers, MemoryBus& bus) override {
    if (from_memory_) {
      if (is_16bit_) {
        bus.Write16(value_, bus.Read16(registers.Get(from_)));
      } else {
        bus.Write8(value_, bus.Read8(registers.Get(from_)));
      }
    } else {
      if (is_16bit_) {
        bus.Write16(value_, registers.Get(from_));
      } else {
        bus.Write8(value_, registers.Get(from_));
      }
    }
    return cycles_;
  }
};

struct InstructionRotateLeftCircular : Instruction {
  ArithmeticTarget to_;
  bool set_zero_;

  int cycles_;

  InstructionRotateLeftCircular()
      : Instruction(InstructionType::RLCA), to_(ArithmeticTarget::A), set_zero_(true), cycles_(4) { }

  InstructionRotateLeftCircular(ArithmeticTarget to, int cycles)
      : Instruction(InstructionType::RLC), to_(to), set_zero_(true), cycles_(cycles) { }

  int Execute(CPU& cpu, Registers &registers, MemoryBus& bus) override {
    u8 a = registers.Get(to_);
    bool carry = a & 0x80;
    a = (a << 1) & 0xFF;
    registers.flags.f.carry = carry;
    a = a | carry;
    registers.Set(to_, a);
    registers.flags.f.zero = set_zero_ && a == 0;
    registers.flags.f.half_carry = false;
    registers.flags.f.subtract = false;
    return cycles_;
  }
};

struct InstructionRotateRightCircular : Instruction {
  ArithmeticTarget to_;
  bool set_zero_;

  int cycles_;

  InstructionRotateRightCircular()
      : Instruction(InstructionType::RRCA), to_(ArithmeticTarget::A), set_zero_(false), cycles_(4) { }

  InstructionRotateRightCircular(ArithmeticTarget to, int cycles)
      : Instruction(InstructionType::RRC), to_(to), set_zero_(true), cycles_(cycles) { }

  int Execute(CPU& cpu, Registers &registers, MemoryBus& bus) override {
    u8 a = registers.Get(to_);
    bool carry = a & 0b000'0001;
    a = (a >> 1) & 0b0111'1111;
    registers.flags.f.carry = carry;
    a = a | (carry << 7);
    registers.Set(to_, a);
    registers.flags.f.zero = set_zero_ && a == 0;
    registers.flags.f.half_carry = false;
    registers.flags.f.subtract = false;
    return cycles_;
  }
};

struct InstructionRotateLeft : Instruction {
  ArithmeticTarget to_;
  bool set_zero_;

  int cycles_;

  InstructionRotateLeft()
      : Instruction(InstructionType::RLA), to_(ArithmeticTarget::A), set_zero_(true), cycles_(4) { }

  InstructionRotateLeft(ArithmeticTarget to, int cycles)
      : Instruction(InstructionType::RL), to_(to), set_zero_(true), cycles_(cycles) { }

  int Execute(CPU& cpu, Registers &registers, MemoryBus& bus) override {
    u8 a = registers.Get(to_);
    bool carry = a & 0b1000'0000;
    a = (a << 1) & 0b1111'1111;
    registers.flags.f.carry = carry;
    registers.Set(to_, a);
    registers.flags.f.zero = set_zero_ && a == 0;
    registers.flags.f.half_carry = false;
    registers.flags.f.subtract = false;
    return cycles_;
  }
};


struct InstructionRotateRight : Instruction {
  ArithmeticTarget to_;
  bool set_zero_;

  int cycles_;

  InstructionRotateRight()
      : Instruction(InstructionType::RRA), to_(ArithmeticTarget::A), set_zero_(true), cycles_(4) { }

  InstructionRotateRight(ArithmeticTarget to, int cycles)
      : Instruction(InstructionType::RR), to_(to), set_zero_(true), cycles_(cycles) { }

  int Execute(CPU& cpu, Registers &registers, MemoryBus& bus) override {
    u8 a = registers.Get(to_);
    bool carry = a & 0b0000'0001;
    a = (a >> 1) & 0b0111'1111;
    registers.flags.f.carry = carry;
    registers.Set(to_, a);
    registers.flags.f.zero = set_zero_ && a == 0;
    registers.flags.f.half_carry = false;
    registers.flags.f.subtract = false;
    return cycles_;
  }
};

struct InstructionStop : Instruction {
  InstructionStop() : Instruction(InstructionType::STOP) { }

  int Execute(CPU& cpu, Registers &registers, MemoryBus& bus) override {
    return 4;
  }
};

struct InstructionInc : Instruction {
  ArithmeticTarget to_;
  int cycles_;

  InstructionInc(ArithmeticTarget to, int cycles)
      : Instruction(InstructionType::INC), to_(to), cycles_(cycles) { }

  int Execute(CPU& cpu, Registers &registers, MemoryBus& bus) override {
    if (is_16bit_register(to_)) {
      u16 result = cpu.Inc16(registers.Get(to_));
      registers.Set(to_, result);
    } else {
      u8 result = cpu.Inc8((u8) registers.Get(to_));
      registers.Set(to_, result);
    }
    return cycles_;
  }
};

struct InstructionDec : Instruction {
  ArithmeticTarget to_;
  int cycles_;

  InstructionDec(ArithmeticTarget to, int cycles)
      : Instruction(InstructionType::DEC), to_(to), cycles_(cycles) { }

  int Execute(CPU& cpu, Registers &registers, MemoryBus& bus) override {
    if (is_16bit_register(to_)) {
      u16 result = cpu.Dec16(registers.Get(to_));
      registers.Set(to_, result);
    } else {
      u8 result = cpu.Dec8((u8) registers.Get(to_));
      registers.Set(to_, result);
    }
    return cycles_;
  }
};

struct InstructionAdd : Instruction {
  ArithmeticTarget to_;
  ArithmeticTarget from_;
  bool from_memory_;

  int cycles_;

  InstructionAdd(ArithmeticTarget to, ArithmeticTarget from, bool from_memory, int cycles)
      : Instruction(InstructionType::ADD), to_(to), from_(from), from_memory_(from_memory), cycles_(cycles) { }

  int Execute(CPU& cpu, Registers &registers, MemoryBus& bus) override {
    u8 value;
    if (from_memory_) {
      value = bus.Read8(registers.Get(from_));
    } else {
      assert(!is_16bit_register(from_));
      value = registers.Get(from_);
    }
    if (is_16bit_register(to_)) {
      u16 result = cpu.Add16(registers.Get(to_), value);
      registers.Set(to_, result);
    } else {
      u8 result = cpu.Add8(registers.Get(to_), value);
      registers.Set(to_, result);
    }
    return cycles_;
  }
};

struct InstructionJump : Instruction {
  s8 value_;

  InstructionJump(s8 value)
      : Instruction(InstructionType::JP), value_(value) { }

  int Execute(CPU& cpu, Registers &registers, MemoryBus& bus) override {
    registers.pc += value_;
    return 12;
  }
};


struct InstructionJumpZero : Instruction {
  s8 value_;
  bool is_not_;

  InstructionJumpZero(s8 value, bool is_not)
      : Instruction(InstructionType::JP), value_(value), is_not_(is_not) { }

  int Execute(CPU& cpu, Registers &registers, MemoryBus& bus) override {
    if (registers.flags.f.zero != is_not_) {
      registers.pc += value_;
      return 12;
    }
    return 8;
  }
};

struct InstructionJumpCarry : Instruction {
  s8 value_;
  bool is_not_;

  InstructionJumpCarry(s8 value, bool is_not)
      : Instruction(InstructionType::JP), value_(value), is_not_(is_not) { }

  int Execute(CPU& cpu, Registers &registers, MemoryBus& bus) override {
    if (registers.flags.f.carry != is_not_) {
      registers.pc += value_;
      return 12;
    }
    return 8;
  }
};

std::unique_ptr<Instruction> Fetch(CPU& cpu, Registers& registers, MemoryBus& bus) {
  u8 opcode = bus.Read8(registers.pc);
  registers.pc++;
  if (opcode == 0x00) { // NOP
    return std::make_unique<InstructionNoOp>();
  } else if (opcode == 0x01) { // LD BC, n16
    u8 value = cpu.Fetch8();
    return std::make_unique<InstructionLoadImmediate>(ArithmeticTarget::BC, false, value, 12);
  } else if (opcode == 0x02) { // LD [BC], A
    return std::make_unique<InstructionLoad>(ArithmeticTarget::BC, true, ArithmeticTarget::A, false, 8);
  } else if (opcode == 0x03) { // INC BC
    return std::make_unique<InstructionInc>(ArithmeticTarget::BC, 8);
  } else if (opcode == 0x04) { // INC B
    return std::make_unique<InstructionInc>(ArithmeticTarget::B, 4);
  } else if (opcode == 0x05) { // DEC B
    return std::make_unique<InstructionDec>(ArithmeticTarget::B, 4);
  } else if (opcode == 0x06) { // LD B, n8
    u8 value = cpu.Fetch8();
    return std::make_unique<InstructionLoadImmediate>(ArithmeticTarget::B, false, value, 8);
  } else if (opcode == 0x07) { // RCLA
    return std::make_unique<InstructionRotateLeftCircular>();
  } else if (opcode == 0x08) { // LD [a16], SP
    u16 value = cpu.Fetch16();
    return std::make_unique<InstructionLoadToAddress>(value, ArithmeticTarget::SP, false, 20);
  } else if (opcode == 0x09) { // ADD HL, BC
    return std::make_unique<InstructionAdd>(ArithmeticTarget::HL, ArithmeticTarget::BC, false, 8);
  } else if (opcode == 0x0A) { // LD A, BC
    return std::make_unique<InstructionLoad>(ArithmeticTarget::A, false, ArithmeticTarget::BC, true, 8);
  } else if (opcode == 0x0B) { // DEC BC
    return std::make_unique<InstructionDec>(ArithmeticTarget::BC, 8);
  } else if (opcode == 0x0C) { // INC C
    return std::make_unique<InstructionInc>(ArithmeticTarget::C, 4);
  } else if (opcode == 0x0D) { // DEC C
    return std::make_unique<InstructionDec>(ArithmeticTarget::C, 4);
  } else if (opcode == 0x0E) { // LD C, n8
    u8 value = cpu.Fetch8();
    return std::make_unique<InstructionLoadImmediate>(ArithmeticTarget::C, false, value, 8);
  } else if (opcode == 0x0F) { // RRCA
    return std::make_unique<InstructionRotateRightCircular>();
  } else if (opcode == 0x10) { // STOP
    cpu.Fetch8();
    return std::make_unique<InstructionStop>();
  } else if (opcode == 0x11) { // LD DE, n16
    u16 value = cpu.Fetch16();
    return std::make_unique<InstructionLoadImmediate>(ArithmeticTarget::DE, false, value, 12);
  } else if (opcode == 0x12) { // LD [DE], a
    return std::make_unique<InstructionLoad>(ArithmeticTarget::DE, true, ArithmeticTarget::A, false, 8);
  } else if (opcode == 0x13) { // INC DE
    return std::make_unique<InstructionInc>(ArithmeticTarget::DE, 8);
  } else if (opcode == 0x14) { // INC D
    return std::make_unique<InstructionInc>(ArithmeticTarget::D, 4);
  } else if (opcode == 0x15) { // DEC D
    return std::make_unique<InstructionInc>(ArithmeticTarget::D, 4);
  } else if (opcode == 0x16) { // LD D, n8
    u8 value = cpu.Fetch8();
    return std::make_unique<InstructionLoadImmediate>(ArithmeticTarget::D, false, value, 8);
  } else if (opcode == 0x17) { // RLA
    return std::make_unique<InstructionRotateLeft>();
  } else if (opcode == 0x18) { // JP e8
    s8 value = as_signed(cpu.Fetch8());
    return std::make_unique<InstructionJump>(value);
  } else if (opcode == 0x19) { // ADD HL, DE
    return std::make_unique<InstructionAdd>(ArithmeticTarget::HL, ArithmeticTarget::DE, false, 8);
  } else if (opcode == 0x1A) { // LD A, [DE]
    return std::make_unique<InstructionLoad>(ArithmeticTarget::A, false, ArithmeticTarget::DE, true, 8);
  } else if (opcode == 0x1B) { // DEC DE
    return std::make_unique<InstructionDec>(ArithmeticTarget::DE, 8);
  } else if (opcode == 0x1C) { // INC E
    return std::make_unique<InstructionInc>(ArithmeticTarget::E, 8);
  }
  return nullptr;
}
