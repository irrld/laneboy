#include "instructions.h"
#include "debug.h"

u8 Fetch(Registers& registers, MemoryBus& bus) {
  u8 v = bus.Read(registers.pc);
  registers.pc = registers.pc + 1;
  return v;
}

u16 FetchWord(Registers& registers, MemoryBus& bus) {
  u16 v = bus.ReadWord(registers.pc);
  registers.pc = registers.pc + 2;
  return v;
}

// Extended ($CB prefixed)
std::unique_ptr<Instruction> FetchPrefixed(ALU& alu, Registers& registers, MemoryBus& bus) {
  // The cycle count of these instructions include the fetching of the prefix value ($CB) as well as
  // the instruction itself
  u16 pc_begin = registers.pc - 1;
  u8 opcode = Fetch(registers, bus);
  if (opcode == 0x00) {  // RLC B
    EMIT_INSTRUCTION(pc_begin, "RLC B");
    return std::make_unique<InstructionRotateLeftCircular>(ArithmeticTarget::B, 8);
  } else if (opcode == 0x01) {  // RLC C
    EMIT_INSTRUCTION(pc_begin, "RLC C");
    return std::make_unique<InstructionRotateLeftCircular>(ArithmeticTarget::C, 8);
  } else if (opcode == 0x02) {  // RLC D
    EMIT_INSTRUCTION(pc_begin, "RLC D");
    return std::make_unique<InstructionRotateLeftCircular>(ArithmeticTarget::D, 8);
  } else if (opcode == 0x03) {  // RLC E
    EMIT_INSTRUCTION(pc_begin, "RLC E");
    return std::make_unique<InstructionRotateLeftCircular>(ArithmeticTarget::E, 8);
  } else if (opcode == 0x04) {  // RLC H
    EMIT_INSTRUCTION(pc_begin, "RLC H");
    return std::make_unique<InstructionRotateLeftCircular>(ArithmeticTarget::H, 8);
  } else if (opcode == 0x05) {  // RLC L
    EMIT_INSTRUCTION(pc_begin, "RLC L");
    return std::make_unique<InstructionRotateLeftCircular>(ArithmeticTarget::L, 8);
  } else if (opcode == 0x06) {  // RLC [HL]
    EMIT_INSTRUCTION(pc_begin, "RLC [HL]");
    return std::make_unique<InstructionRotateLeftCircular>(ArithmeticTarget::HL, true, 16);
  } else if (opcode == 0x07) {  // RLC A
    EMIT_INSTRUCTION(pc_begin, "RLC A");
    return std::make_unique<InstructionRotateLeftCircular>(ArithmeticTarget::A, 8);
  } else if (opcode == 0x08) {  // RRC B
    EMIT_INSTRUCTION(pc_begin, "RRC B");
    return std::make_unique<InstructionRotateRightCircular>(ArithmeticTarget::B, 8);
  } else if (opcode == 0x09) {  // RRC C
    EMIT_INSTRUCTION(pc_begin, "RRC C");
    return std::make_unique<InstructionRotateRightCircular>(ArithmeticTarget::C, 8);
  } else if (opcode == 0x0A) {  // RRC D
    EMIT_INSTRUCTION(pc_begin, "RRC D");
    return std::make_unique<InstructionRotateRightCircular>(ArithmeticTarget::D, 8);
  } else if (opcode == 0x0B) {  // RRC E
    EMIT_INSTRUCTION(pc_begin, "RRC E");
    return std::make_unique<InstructionRotateRightCircular>(ArithmeticTarget::E, 8);
  } else if (opcode == 0x0C) {  // RRC H
    EMIT_INSTRUCTION(pc_begin, "RRC H");
    return std::make_unique<InstructionRotateRightCircular>(ArithmeticTarget::H, 8);
  } else if (opcode == 0x0D) {  // RRC L
    EMIT_INSTRUCTION(pc_begin, "RRC L");
    return std::make_unique<InstructionRotateRightCircular>(ArithmeticTarget::L, 8);
  } else if (opcode == 0x0E) {  // RRC [HL]
    EMIT_INSTRUCTION(pc_begin, "RRC [Hl]");
    return std::make_unique<InstructionRotateRightCircular>(ArithmeticTarget::HL, true, 16);
  } else if (opcode == 0x0F) {  // RRC A
    EMIT_INSTRUCTION(pc_begin, "RRC A");
    return std::make_unique<InstructionRotateRightCircular>(ArithmeticTarget::A, 8);
  } else if (opcode == 0x10) {  // RL B
    EMIT_INSTRUCTION(pc_begin, "RL B");
    return std::make_unique<InstructionRotateLeft>(ArithmeticTarget::B, 8);
  } else if (opcode == 0x11) {  // RL C
    EMIT_INSTRUCTION(pc_begin, "RL C");
    return std::make_unique<InstructionRotateLeft>(ArithmeticTarget::C, 8);
  } else if (opcode == 0x12) {  // RL D
    EMIT_INSTRUCTION(pc_begin, "RL D");
    return std::make_unique<InstructionRotateLeft>(ArithmeticTarget::D, 8);
  } else if (opcode == 0x13) {  // RL E
    EMIT_INSTRUCTION(pc_begin, "RL E");
    return std::make_unique<InstructionRotateLeft>(ArithmeticTarget::E, 8);
  } else if (opcode == 0x14) {  // RL H
    EMIT_INSTRUCTION(pc_begin, "RL H");
    return std::make_unique<InstructionRotateLeft>(ArithmeticTarget::H, 8);
  } else if (opcode == 0x15) {  // RL L
    EMIT_INSTRUCTION(pc_begin, "RL L");
    return std::make_unique<InstructionRotateLeft>(ArithmeticTarget::L, 8);
  } else if (opcode == 0x16) {  // RL [HL]
    EMIT_INSTRUCTION(pc_begin, "RL [HL]");
    return std::make_unique<InstructionRotateLeft>(ArithmeticTarget::HL, true, 16);
  } else if (opcode == 0x17) {  // RL A
    EMIT_INSTRUCTION(pc_begin, "RL A");
    return std::make_unique<InstructionRotateLeft>(ArithmeticTarget::A, 8);
  } else if (opcode == 0x18) {  // RR B
    EMIT_INSTRUCTION(pc_begin, "RR B");
    return std::make_unique<InstructionRotateRight>(ArithmeticTarget::B, 8);
  } else if (opcode == 0x19) {  // RR C
    EMIT_INSTRUCTION(pc_begin, "RR C");
    return std::make_unique<InstructionRotateRight>(ArithmeticTarget::C, 8);
  } else if (opcode == 0x1A) {  // RR D
    EMIT_INSTRUCTION(pc_begin, "RR D");
    return std::make_unique<InstructionRotateRight>(ArithmeticTarget::D, 8);
  } else if (opcode == 0x1B) {  // RR E
    EMIT_INSTRUCTION(pc_begin, "RR E");
    return std::make_unique<InstructionRotateRight>(ArithmeticTarget::E, 8);
  } else if (opcode == 0x1C) {  // RR H
    EMIT_INSTRUCTION(pc_begin, "RR H");
    return std::make_unique<InstructionRotateRight>(ArithmeticTarget::H, 8);
  } else if (opcode == 0x1D) {  // RR L
    EMIT_INSTRUCTION(pc_begin, "RR L");
    return std::make_unique<InstructionRotateRight>(ArithmeticTarget::L, 8);
  } else if (opcode == 0x1E) {  // RR [HL]
    EMIT_INSTRUCTION(pc_begin, "RR [HL]");
    return std::make_unique<InstructionRotateRight>(ArithmeticTarget::HL, true, 16);
  } else if (opcode == 0x1F) {  // RR A
    EMIT_INSTRUCTION(pc_begin, "RR A");
    return std::make_unique<InstructionRotateRight>(ArithmeticTarget::A, 8);
  } else if (opcode == 0x20) {  // SLA B
    EMIT_INSTRUCTION(pc_begin, "SLA B");
    return std::make_unique<InstructionShiftLeftArithmetic>(ArithmeticTarget::B, 8);
  } else if (opcode == 0x21) {  // SLA C
    EMIT_INSTRUCTION(pc_begin, "SLA C");
    return std::make_unique<InstructionShiftLeftArithmetic>(ArithmeticTarget::C, 8);
  } else if (opcode == 0x22) {  // SLA D
    EMIT_INSTRUCTION(pc_begin, "SLA D");
    return std::make_unique<InstructionShiftLeftArithmetic>(ArithmeticTarget::D, 8);
  } else if (opcode == 0x23) {  // SLA E
    EMIT_INSTRUCTION(pc_begin, "SLA E");
    return std::make_unique<InstructionShiftLeftArithmetic>(ArithmeticTarget::E, 8);
  } else if (opcode == 0x24) {  // SLA H
    EMIT_INSTRUCTION(pc_begin, "SLA H");
    return std::make_unique<InstructionShiftLeftArithmetic>(ArithmeticTarget::H, 8);
  } else if (opcode == 0x25) {  // SLA L
    EMIT_INSTRUCTION(pc_begin, "SLA L");
    return std::make_unique<InstructionShiftLeftArithmetic>(ArithmeticTarget::L, 8);
  } else if (opcode == 0x26) {  // SLA [HL]
    EMIT_INSTRUCTION(pc_begin, "SLA [HL]");
    return std::make_unique<InstructionShiftLeftArithmetic>(ArithmeticTarget::HL, true, 16);
  } else if (opcode == 0x27) {  // SLA A
    EMIT_INSTRUCTION(pc_begin, "SLA A");
    return std::make_unique<InstructionShiftLeftArithmetic>(ArithmeticTarget::A, 8);
  } else if (opcode == 0x28) {  // SRA B
    EMIT_INSTRUCTION(pc_begin, "SRA B");
    return std::make_unique<InstructionShiftRightArithmetic>(ArithmeticTarget::B, 8);
  } else if (opcode == 0x29) {  // SRA C
    EMIT_INSTRUCTION(pc_begin, "SRA C");
    return std::make_unique<InstructionShiftRightArithmetic>(ArithmeticTarget::C, 8);
  } else if (opcode == 0x2A) {  // SRA D
    EMIT_INSTRUCTION(pc_begin, "SRA D");
    return std::make_unique<InstructionShiftRightArithmetic>(ArithmeticTarget::D, 8);
  } else if (opcode == 0x2B) {  // SRA E
    EMIT_INSTRUCTION(pc_begin, "SRA E");
    return std::make_unique<InstructionShiftRightArithmetic>(ArithmeticTarget::E, 8);
  } else if (opcode == 0x2C) {  // SRA H
    EMIT_INSTRUCTION(pc_begin, "SRA H");
    return std::make_unique<InstructionShiftRightArithmetic>(ArithmeticTarget::H, 8);
  } else if (opcode == 0x2D) {  // SRA L
    EMIT_INSTRUCTION(pc_begin, "SRA L");
    return std::make_unique<InstructionShiftRightArithmetic>(ArithmeticTarget::L, 8);
  } else if (opcode == 0x2E) {  // SRA [HL]
    EMIT_INSTRUCTION(pc_begin, "SRA [HL]");
    return std::make_unique<InstructionShiftRightArithmetic>(ArithmeticTarget::HL, true, 16);
  } else if (opcode == 0x2F) {  // SRA A
    EMIT_INSTRUCTION(pc_begin, "SRA A");
    return std::make_unique<InstructionShiftRightArithmetic>(ArithmeticTarget::A, 8);
  } else if (opcode == 0x30) {  // SWAP B
    EMIT_INSTRUCTION(pc_begin, "SWAP B");
    return std::make_unique<InstructionSwap>(ArithmeticTarget::B, 8);
  } else if (opcode == 0x31) {  // SWAP C
    EMIT_INSTRUCTION(pc_begin, "SWAP C");
    return std::make_unique<InstructionSwap>(ArithmeticTarget::C, 8);
  } else if (opcode == 0x32) {  // SWAP D
    EMIT_INSTRUCTION(pc_begin, "SWAP D");
    return std::make_unique<InstructionSwap>(ArithmeticTarget::D, 8);
  } else if (opcode == 0x33) {  // SWAP E
    EMIT_INSTRUCTION(pc_begin, "SWAP E");
    return std::make_unique<InstructionSwap>(ArithmeticTarget::E, 8);
  } else if (opcode == 0x34) {  // SWAP H
    EMIT_INSTRUCTION(pc_begin, "SWAP H");
    return std::make_unique<InstructionSwap>(ArithmeticTarget::H, 8);
  } else if (opcode == 0x35) {  // SWAP L
    EMIT_INSTRUCTION(pc_begin, "SWAP L");
    return std::make_unique<InstructionSwap>(ArithmeticTarget::L, 8);
  } else if (opcode == 0x36) {  // SWAP [HL]
    EMIT_INSTRUCTION(pc_begin, "SWAP [HL]");
    return std::make_unique<InstructionSwap>(ArithmeticTarget::HL, true, 16);
  } else if (opcode == 0x37) {  // SWAP A
    EMIT_INSTRUCTION(pc_begin, "SWAP A");
    return std::make_unique<InstructionSwap>(ArithmeticTarget::A, 8);
  } else if (opcode == 0x38) {  // SRL B
    EMIT_INSTRUCTION(pc_begin, "SRL B");
    return std::make_unique<InstructionShiftRightLogical>(ArithmeticTarget::B, 8);
  } else if (opcode == 0x39) {  // SRL C
    EMIT_INSTRUCTION(pc_begin, "SRL C");
    return std::make_unique<InstructionShiftRightLogical>(ArithmeticTarget::C, 8);
  } else if (opcode == 0x3A) {  // SRL D
    EMIT_INSTRUCTION(pc_begin, "SRL D");
    return std::make_unique<InstructionShiftRightLogical>(ArithmeticTarget::D, 8);
  } else if (opcode == 0x3B) {  // SRL E
    EMIT_INSTRUCTION(pc_begin, "SRL E");
    return std::make_unique<InstructionShiftRightLogical>(ArithmeticTarget::E, 8);
  } else if (opcode == 0x3C) {  // SRL H
    EMIT_INSTRUCTION(pc_begin, "SRL H");
    return std::make_unique<InstructionShiftRightLogical>(ArithmeticTarget::H, 8);
  } else if (opcode == 0x3D) {  // SRL L
    EMIT_INSTRUCTION(pc_begin, "SRL L");
    return std::make_unique<InstructionShiftRightLogical>(ArithmeticTarget::L, 8);
  } else if (opcode == 0x3E) {  // SRL [HL]
    EMIT_INSTRUCTION(pc_begin, "SRL [HL]");
    return std::make_unique<InstructionShiftRightLogical>(ArithmeticTarget::HL, true, 16);
  } else if (opcode == 0x3F) {  // SRL A
    EMIT_INSTRUCTION(pc_begin, "SRL A");
    return std::make_unique<InstructionShiftRightLogical>(ArithmeticTarget::A, 8);
  } else if (opcode == 0x40) {  // BIT 0, B
    EMIT_INSTRUCTION(pc_begin, "BIT 0, B");
    return std::make_unique<InstructionBit>(0, ArithmeticTarget::B, 8);
  } else if (opcode == 0x41) {  // BIT 0, C
    EMIT_INSTRUCTION(pc_begin, "BIT 0, C");
    return std::make_unique<InstructionBit>(0, ArithmeticTarget::C, 8);
  } else if (opcode == 0x42) {  // BIT 0, D
    EMIT_INSTRUCTION(pc_begin, "BIT 0, D");
    return std::make_unique<InstructionBit>(0, ArithmeticTarget::D, 8);
  } else if (opcode == 0x43) {  // BIT 0, E
    EMIT_INSTRUCTION(pc_begin, "BIT 0, E");
    return std::make_unique<InstructionBit>(0, ArithmeticTarget::E, 8);
  } else if (opcode == 0x44) {  // BIT 0, H
    EMIT_INSTRUCTION(pc_begin, "BIT 0, H");
    return std::make_unique<InstructionBit>(0, ArithmeticTarget::H, 8);
  } else if (opcode == 0x45) {  // BIT 0, L
    EMIT_INSTRUCTION(pc_begin, "BIT 0, L");
    return std::make_unique<InstructionBit>(0, ArithmeticTarget::L, 8);
  } else if (opcode == 0x46) {  // BIT 0, [HL]
    EMIT_INSTRUCTION(pc_begin, "BIT 0, [HL]");
    return std::make_unique<InstructionBit>(0, ArithmeticTarget::HL, true, 12);
  } else if (opcode == 0x47) {  // BIT 0, A
    EMIT_INSTRUCTION(pc_begin, "BIT 0, A");
    return std::make_unique<InstructionBit>(0, ArithmeticTarget::A, 8);
  } else if (opcode == 0x48) {  // BIT 1, B
    EMIT_INSTRUCTION(pc_begin, "BIT 1, B");
    return std::make_unique<InstructionBit>(1, ArithmeticTarget::B, 8);
  } else if (opcode == 0x49) {  // BIT 1, C
    EMIT_INSTRUCTION(pc_begin, "BIT 1, C");
    return std::make_unique<InstructionBit>(1, ArithmeticTarget::C, 8);
  } else if (opcode == 0x4A) {  // BIT 1, D
    EMIT_INSTRUCTION(pc_begin, "BIT 1, D");
    return std::make_unique<InstructionBit>(1, ArithmeticTarget::D, 8);
  } else if (opcode == 0x4B) {  // BIT 1, E
    EMIT_INSTRUCTION(pc_begin, "BIT 1, E");
    return std::make_unique<InstructionBit>(1, ArithmeticTarget::E, 8);
  } else if (opcode == 0x4C) {  // BIT 1, H
    EMIT_INSTRUCTION(pc_begin, "BIT 1, H");
    return std::make_unique<InstructionBit>(1, ArithmeticTarget::H, 8);
  } else if (opcode == 0x4D) {  // BIT 1, L
    EMIT_INSTRUCTION(pc_begin, "BIT 1, L");
    return std::make_unique<InstructionBit>(1, ArithmeticTarget::L, 8);
  } else if (opcode == 0x4E) {  // BIT 1, [HL]
    EMIT_INSTRUCTION(pc_begin, "BIT 1, [HL]");
    return std::make_unique<InstructionBit>(1, ArithmeticTarget::HL, true, 12);
  } else if (opcode == 0x4F) {  // BIT 1, A
    EMIT_INSTRUCTION(pc_begin, "BIT 1, A");
    return std::make_unique<InstructionBit>(1, ArithmeticTarget::A, 8);
  } else if (opcode == 0x50) {  // BIT 2, B
    EMIT_INSTRUCTION(pc_begin, "BIT 2, B");
    return std::make_unique<InstructionBit>(2, ArithmeticTarget::B, 8);
  } else if (opcode == 0x51) {  // BIT 2, C
    EMIT_INSTRUCTION(pc_begin, "BIT 2, C");
    return std::make_unique<InstructionBit>(2, ArithmeticTarget::C, 8);
  } else if (opcode == 0x52) {  // BIT 2, D
    EMIT_INSTRUCTION(pc_begin, "BIT 2, D");
    return std::make_unique<InstructionBit>(2, ArithmeticTarget::D, 8);
  } else if (opcode == 0x53) {  // BIT 2, E
    EMIT_INSTRUCTION(pc_begin, "BIT 2, E");
    return std::make_unique<InstructionBit>(2, ArithmeticTarget::E, 8);
  } else if (opcode == 0x54) {  // BIT 2, H
    EMIT_INSTRUCTION(pc_begin, "BIT 2, H");
    return std::make_unique<InstructionBit>(2, ArithmeticTarget::H, 8);
  } else if (opcode == 0x55) {  // BIT 2, L
    EMIT_INSTRUCTION(pc_begin, "BIT 2, L");
    return std::make_unique<InstructionBit>(2, ArithmeticTarget::L, 8);
  } else if (opcode == 0x56) {  // BIT 2, [HL]
    EMIT_INSTRUCTION(pc_begin, "BIT 2, [HL]");
    return std::make_unique<InstructionBit>(2, ArithmeticTarget::HL, true, 12);
  } else if (opcode == 0x57) {  // BIT 2, A
    EMIT_INSTRUCTION(pc_begin, "BIT 2, A");
    return std::make_unique<InstructionBit>(2, ArithmeticTarget::A, 8);
  } else if (opcode == 0x58) {  // BIT 3, B
    EMIT_INSTRUCTION(pc_begin, "BIT 3, B");
    return std::make_unique<InstructionBit>(3, ArithmeticTarget::B, 8);
  } else if (opcode == 0x59) {  // BIT 3, C
    EMIT_INSTRUCTION(pc_begin, "BIT 3, C");
    return std::make_unique<InstructionBit>(3, ArithmeticTarget::C, 8);
  } else if (opcode == 0x5A) {  // BIT 3, D
    EMIT_INSTRUCTION(pc_begin, "BIT 3, D");
    return std::make_unique<InstructionBit>(3, ArithmeticTarget::D, 8);
  } else if (opcode == 0x5B) {  // BIT 3, E
    EMIT_INSTRUCTION(pc_begin, "BIT 3, E");
    return std::make_unique<InstructionBit>(3, ArithmeticTarget::E, 8);
  } else if (opcode == 0x5C) {  // BIT 3, H
    EMIT_INSTRUCTION(pc_begin, "BIT 3, H");
    return std::make_unique<InstructionBit>(3, ArithmeticTarget::H, 8);
  } else if (opcode == 0x5D) {  // BIT 3, L
    EMIT_INSTRUCTION(pc_begin, "BIT 3, L");
    return std::make_unique<InstructionBit>(3, ArithmeticTarget::L, 8);
  } else if (opcode == 0x5E) {  // BIT 3, [HL]
    EMIT_INSTRUCTION(pc_begin, "BIT 3, [HL]");
    return std::make_unique<InstructionBit>(3, ArithmeticTarget::HL, true, 12);
  } else if (opcode == 0x5F) {  // BIT 3, A
    EMIT_INSTRUCTION(pc_begin, "BIT 3, A");
    return std::make_unique<InstructionBit>(3, ArithmeticTarget::A, 8);
  } else if (opcode == 0x60) {  // BIT 4, B
    EMIT_INSTRUCTION(pc_begin, "BIT 4, B");
    return std::make_unique<InstructionBit>(4, ArithmeticTarget::B, 8);
  } else if (opcode == 0x61) {  // BIT 4, C
    EMIT_INSTRUCTION(pc_begin, "BIT 4, C");
    return std::make_unique<InstructionBit>(4, ArithmeticTarget::C, 8);
  } else if (opcode == 0x62) {  // BIT 4, D
    EMIT_INSTRUCTION(pc_begin, "BIT 4, D");
    return std::make_unique<InstructionBit>(4, ArithmeticTarget::D, 8);
  } else if (opcode == 0x63) {  // BIT 4, E
    EMIT_INSTRUCTION(pc_begin, "BIT 4, E");
    return std::make_unique<InstructionBit>(4, ArithmeticTarget::E, 8);
  } else if (opcode == 0x64) {  // BIT 4, H
    EMIT_INSTRUCTION(pc_begin, "BIT 4, L");
    return std::make_unique<InstructionBit>(4, ArithmeticTarget::H, 8);
  } else if (opcode == 0x65) {  // BIT 4, L
    EMIT_INSTRUCTION(pc_begin, "BIT 4, L");
    return std::make_unique<InstructionBit>(4, ArithmeticTarget::L, 8);
  } else if (opcode == 0x66) {  // BIT 4, [HL]
    EMIT_INSTRUCTION(pc_begin, "BIT 4, [HL]");
    return std::make_unique<InstructionBit>(4, ArithmeticTarget::HL, true, 12);
  } else if (opcode == 0x67) {  // BIT 4, A
    EMIT_INSTRUCTION(pc_begin, "BIT 4, A");
    return std::make_unique<InstructionBit>(4, ArithmeticTarget::A, 8);
  } else if (opcode == 0x68) {  // BIT 5, B
    EMIT_INSTRUCTION(pc_begin, "BIT 5, B");
    return std::make_unique<InstructionBit>(5, ArithmeticTarget::B, 8);
  } else if (opcode == 0x69) {  // BIT 5, C
    EMIT_INSTRUCTION(pc_begin, "BIT 5, C");
    return std::make_unique<InstructionBit>(5, ArithmeticTarget::C, 8);
  } else if (opcode == 0x6A) {  // BIT 5, D
    EMIT_INSTRUCTION(pc_begin, "BIT 5, D");
    return std::make_unique<InstructionBit>(5, ArithmeticTarget::D, 8);
  } else if (opcode == 0x6B) {  // BIT 5, E
    EMIT_INSTRUCTION(pc_begin, "BIT 5, E");
    return std::make_unique<InstructionBit>(5, ArithmeticTarget::E, 8);
  } else if (opcode == 0x6C) {  // BIT 5, H
    EMIT_INSTRUCTION(pc_begin, "BIT 5, H");
    return std::make_unique<InstructionBit>(5, ArithmeticTarget::H, 8);
  } else if (opcode == 0x6D) {  // BIT 5, L
    EMIT_INSTRUCTION(pc_begin, "BIT 5, L");
    return std::make_unique<InstructionBit>(5, ArithmeticTarget::L, 8);
  } else if (opcode == 0x6E) {  // BIT 5, [HL]
    EMIT_INSTRUCTION(pc_begin, "BIT 5, [HL]");
    return std::make_unique<InstructionBit>(5, ArithmeticTarget::HL, true, 12);
  } else if (opcode == 0x6F) {  // BIT 5, A
    EMIT_INSTRUCTION(pc_begin, "BIT 5, A");
    return std::make_unique<InstructionBit>(5, ArithmeticTarget::A, 8);
  } else if (opcode == 0x70) {  // BIT 6, B
    EMIT_INSTRUCTION(pc_begin, "BIT 6, B");
    return std::make_unique<InstructionBit>(6, ArithmeticTarget::B, 8);
  } else if (opcode == 0x71) {  // BIT 6, C
    EMIT_INSTRUCTION(pc_begin, "BIT 6, C");
    return std::make_unique<InstructionBit>(6, ArithmeticTarget::C, 8);
  } else if (opcode == 0x72) {  // BIT 6, D
    EMIT_INSTRUCTION(pc_begin, "BIT 6, D");
    return std::make_unique<InstructionBit>(6, ArithmeticTarget::D, 8);
  } else if (opcode == 0x73) {  // BIT 6, E
    EMIT_INSTRUCTION(pc_begin, "BIT 6, E");
    return std::make_unique<InstructionBit>(6, ArithmeticTarget::E, 8);
  } else if (opcode == 0x74) {  // BIT 6, H
    EMIT_INSTRUCTION(pc_begin, "BIT 6, H");
    return std::make_unique<InstructionBit>(6, ArithmeticTarget::H, 8);
  } else if (opcode == 0x75) {  // BIT 6, L
    EMIT_INSTRUCTION(pc_begin, "BIT 6, L");
    return std::make_unique<InstructionBit>(6, ArithmeticTarget::L, 8);
  } else if (opcode == 0x76) {  // BIT 6, [HL]
    EMIT_INSTRUCTION(pc_begin, "BIT 6, [HL]");
    return std::make_unique<InstructionBit>(6, ArithmeticTarget::HL, true, 12);
  } else if (opcode == 0x77) {  // BIT 6, A
    EMIT_INSTRUCTION(pc_begin, "BIT 6, A");
    return std::make_unique<InstructionBit>(6, ArithmeticTarget::A, 8);
  } else if (opcode == 0x78) {  // BIT 7, B
    EMIT_INSTRUCTION(pc_begin, "BIT 7, B");
    return std::make_unique<InstructionBit>(7, ArithmeticTarget::B, 8);
  } else if (opcode == 0x79) {  // BIT 7, C
    EMIT_INSTRUCTION(pc_begin, "BIT 7, c");
    return std::make_unique<InstructionBit>(7, ArithmeticTarget::C, 8);
  } else if (opcode == 0x7A) {  // BIT 7, D
    EMIT_INSTRUCTION(pc_begin, "BIT 7, D");
    return std::make_unique<InstructionBit>(7, ArithmeticTarget::D, 8);
  } else if (opcode == 0x7B) {  // BIT 7, E
    EMIT_INSTRUCTION(pc_begin, "BIT 7, E");
    return std::make_unique<InstructionBit>(7, ArithmeticTarget::E, 8);
  } else if (opcode == 0x7C) {  // BIT 7, H
    EMIT_INSTRUCTION(pc_begin, "BIT 7, H");
    return std::make_unique<InstructionBit>(7, ArithmeticTarget::H, 8);
  } else if (opcode == 0x7D) {  // BIT 7, L
    EMIT_INSTRUCTION(pc_begin, "BIT 7, L");
    return std::make_unique<InstructionBit>(7, ArithmeticTarget::L, 8);
  } else if (opcode == 0x7E) {  // BIT 7, [HL]
    EMIT_INSTRUCTION(pc_begin, "BIT 7, [HL]");
    return std::make_unique<InstructionBit>(7, ArithmeticTarget::HL, true, 12);
  } else if (opcode == 0x7F) {  // BIT 7, A
    EMIT_INSTRUCTION(pc_begin, "BIT 7, A");
    return std::make_unique<InstructionBit>(7, ArithmeticTarget::A, 8);
  } else if (opcode == 0x80) {  // RES 0, B
    EMIT_INSTRUCTION(pc_begin, "RES 0, B");
    return std::make_unique<InstructionRes>(0, ArithmeticTarget::B, 8);
  } else if (opcode == 0x81) {  // RES 0, C
    EMIT_INSTRUCTION(pc_begin, "RES 0, C");
    return std::make_unique<InstructionRes>(0, ArithmeticTarget::C, 8);
  } else if (opcode == 0x82) {  // RES 0, D
    EMIT_INSTRUCTION(pc_begin, "RES 0, D");
    return std::make_unique<InstructionRes>(0, ArithmeticTarget::D, 8);
  } else if (opcode == 0x83) {  // RES 0, E
    EMIT_INSTRUCTION(pc_begin, "RES 0, E");
    return std::make_unique<InstructionRes>(0, ArithmeticTarget::E, 8);
  } else if (opcode == 0x84) {  // RES 0, H
    EMIT_INSTRUCTION(pc_begin, "RES 0, H");
    return std::make_unique<InstructionRes>(0, ArithmeticTarget::H, 8);
  } else if (opcode == 0x85) {  // RES 0, L
    EMIT_INSTRUCTION(pc_begin, "RES 0, L");
    return std::make_unique<InstructionRes>(0, ArithmeticTarget::L, 8);
  } else if (opcode == 0x86) {  // RES 0, [HL]
    EMIT_INSTRUCTION(pc_begin, "RES 0, [HL]");
    return std::make_unique<InstructionRes>(0, ArithmeticTarget::HL, true, 16);
  } else if (opcode == 0x87) {  // RES 0, A
    EMIT_INSTRUCTION(pc_begin, "RES 0, A");
    return std::make_unique<InstructionRes>(0, ArithmeticTarget::A, 8);
  } else if (opcode == 0x88) {  // RES 1, B
    EMIT_INSTRUCTION(pc_begin, "RES 1, B");
    return std::make_unique<InstructionRes>(1, ArithmeticTarget::B, 8);
  } else if (opcode == 0x89) {  // RES 1, C
    EMIT_INSTRUCTION(pc_begin, "RES 1, C");
    return std::make_unique<InstructionRes>(1, ArithmeticTarget::C, 8);
  } else if (opcode == 0x8A) {  // RES 1, D
    EMIT_INSTRUCTION(pc_begin, "RES 1, D");
    return std::make_unique<InstructionRes>(1, ArithmeticTarget::D, 8);
  } else if (opcode == 0x8B) {  // RES 1, E
    EMIT_INSTRUCTION(pc_begin, "RES 1, E");
    return std::make_unique<InstructionRes>(1, ArithmeticTarget::E, 8);
  } else if (opcode == 0x8C) {  // RES 1, H
    EMIT_INSTRUCTION(pc_begin, "RES 1, H");
    return std::make_unique<InstructionRes>(1, ArithmeticTarget::H, 8);
  } else if (opcode == 0x8D) {  // RES 1, L
    EMIT_INSTRUCTION(pc_begin, "RES 1, L");
    return std::make_unique<InstructionRes>(1, ArithmeticTarget::L, 8);
  } else if (opcode == 0x8E) {  // RES 1, [HL]
    EMIT_INSTRUCTION(pc_begin, "RES 1, [HL]");
    return std::make_unique<InstructionRes>(1, ArithmeticTarget::HL, true, 16);
  } else if (opcode == 0x8F) {  // RES 1, A
    EMIT_INSTRUCTION(pc_begin, "RES 1, A");
    return std::make_unique<InstructionRes>(1, ArithmeticTarget::A, 8);
  } else if (opcode == 0x90) {  // RES 2, B
    EMIT_INSTRUCTION(pc_begin, "RES 2, B");
    return std::make_unique<InstructionRes>(2, ArithmeticTarget::B, 8);
  } else if (opcode == 0x91) {  // RES 2, C
    EMIT_INSTRUCTION(pc_begin, "RES 2, C");
    return std::make_unique<InstructionRes>(2, ArithmeticTarget::C, 8);
  } else if (opcode == 0x92) {  // RES 2, D
    EMIT_INSTRUCTION(pc_begin, "RES 2, D");
    return std::make_unique<InstructionRes>(2, ArithmeticTarget::D, 8);
  } else if (opcode == 0x93) {  // RES 2, E
    EMIT_INSTRUCTION(pc_begin, "RES 2, E");
    return std::make_unique<InstructionRes>(2, ArithmeticTarget::E, 8);
  } else if (opcode == 0x94) {  // RES 2, H
    EMIT_INSTRUCTION(pc_begin, "RES 2, H");
    return std::make_unique<InstructionRes>(2, ArithmeticTarget::H, 8);
  } else if (opcode == 0x95) {  // RES 2, L
    EMIT_INSTRUCTION(pc_begin, "RES 2, L");
    return std::make_unique<InstructionRes>(2, ArithmeticTarget::L, 8);
  } else if (opcode == 0x96) {  // RES 2, [HL]
    EMIT_INSTRUCTION(pc_begin, "RES 2, [HL]");
    return std::make_unique<InstructionRes>(2, ArithmeticTarget::HL, true, 16);
  } else if (opcode == 0x97) {  // RES 2, A
    EMIT_INSTRUCTION(pc_begin, "RES 2, A");
    return std::make_unique<InstructionRes>(2, ArithmeticTarget::A, 8);
  } else if (opcode == 0x98) {  // RES 3, B
    EMIT_INSTRUCTION(pc_begin, "RES 3, B");
    return std::make_unique<InstructionRes>(3, ArithmeticTarget::B, 8);
  } else if (opcode == 0x99) {  // RES 3, C
    EMIT_INSTRUCTION(pc_begin, "RES 3, C");
    return std::make_unique<InstructionRes>(3, ArithmeticTarget::C, 8);
  } else if (opcode == 0x9A) {  // RES 3, D
    EMIT_INSTRUCTION(pc_begin, "RES 3, D");
    return std::make_unique<InstructionRes>(3, ArithmeticTarget::D, 8);
  } else if (opcode == 0X9B) {  // RES 3, E
    EMIT_INSTRUCTION(pc_begin, "RES 3, E");
    return std::make_unique<InstructionRes>(3, ArithmeticTarget::E, 8);
  } else if (opcode == 0x9C) {  // RES 3, H
    EMIT_INSTRUCTION(pc_begin, "RES 3, H");
    return std::make_unique<InstructionRes>(3, ArithmeticTarget::H, 8);
  } else if (opcode == 0x9D) {  // RES 3, L
    EMIT_INSTRUCTION(pc_begin, "RES 3, L");
    return std::make_unique<InstructionRes>(3, ArithmeticTarget::L, 8);
  } else if (opcode == 0x9E) {  // RES 3, [HL]
    EMIT_INSTRUCTION(pc_begin, "RES 3, [HL]");
    return std::make_unique<InstructionRes>(3, ArithmeticTarget::HL, true, 16);
  } else if (opcode == 0x9F) {  // RES 3, A
    EMIT_INSTRUCTION(pc_begin, "RES 3, A");
    return std::make_unique<InstructionRes>(3, ArithmeticTarget::A, 8);
  } else if (opcode == 0xA0) {  // RES 4, B
    EMIT_INSTRUCTION(pc_begin, "RES 4, B");
    return std::make_unique<InstructionRes>(4, ArithmeticTarget::B, 8);
  } else if (opcode == 0xA1) {  // RES 4, C
    EMIT_INSTRUCTION(pc_begin, "RES 4, C");
    return std::make_unique<InstructionRes>(4, ArithmeticTarget::C, 8);
  } else if (opcode == 0xA2) {  // RES 4, D
    EMIT_INSTRUCTION(pc_begin, "RES 4, D");
    return std::make_unique<InstructionRes>(4, ArithmeticTarget::D, 8);
  } else if (opcode == 0xA3) {  // RES 4, E
    EMIT_INSTRUCTION(pc_begin, "RES 4, E");
    return std::make_unique<InstructionRes>(4, ArithmeticTarget::E, 8);
  } else if (opcode == 0xA4) {  // RES 4, H
    EMIT_INSTRUCTION(pc_begin, "RES 4, H");
    return std::make_unique<InstructionRes>(4, ArithmeticTarget::H, 8);
  } else if (opcode == 0xA5) {  // RES 4, L
    EMIT_INSTRUCTION(pc_begin, "RES 4, L");
    return std::make_unique<InstructionRes>(4, ArithmeticTarget::L, 8);
  } else if (opcode == 0xA6) {  // RES 4, [HL]
    EMIT_INSTRUCTION(pc_begin, "RES 4, [HL]");
    return std::make_unique<InstructionRes>(4, ArithmeticTarget::HL, true, 16);
  } else if (opcode == 0xA7) {  // RES 4, A
    EMIT_INSTRUCTION(pc_begin, "RES 4, A");
    return std::make_unique<InstructionRes>(4, ArithmeticTarget::A, 8);
  } else if (opcode == 0xA8) {  // RES 5, B
    EMIT_INSTRUCTION(pc_begin, "RES 5, B");
    return std::make_unique<InstructionRes>(5, ArithmeticTarget::B, 8);
  } else if (opcode == 0xA9) {  // RES 5, C
    EMIT_INSTRUCTION(pc_begin, "RES 5, C");
    return std::make_unique<InstructionRes>(5, ArithmeticTarget::C, 8);
  } else if (opcode == 0xAA) {  // RES 5, D
    EMIT_INSTRUCTION(pc_begin, "RES 5, D");
    return std::make_unique<InstructionRes>(5, ArithmeticTarget::D, 8);
  } else if (opcode == 0xAB) {  // RES 5, E
    EMIT_INSTRUCTION(pc_begin, "RES 5, E");
    return std::make_unique<InstructionRes>(5, ArithmeticTarget::E, 8);
  } else if (opcode == 0xAC) {  // RES 5, H
    EMIT_INSTRUCTION(pc_begin, "RES 5, H");
    return std::make_unique<InstructionRes>(5, ArithmeticTarget::H, 8);
  } else if (opcode == 0xAD) {  // RES 5, L
    EMIT_INSTRUCTION(pc_begin, "RES 5, L");
    return std::make_unique<InstructionRes>(5, ArithmeticTarget::L, 8);
  } else if (opcode == 0xAE) {  // RES 5, [HL]
    EMIT_INSTRUCTION(pc_begin, "RES 5, [HL]");
    return std::make_unique<InstructionRes>(5, ArithmeticTarget::HL, true, 16);
  } else if (opcode == 0xAF) {  // RES 5, A
    EMIT_INSTRUCTION(pc_begin, "RES 5, A");
    return std::make_unique<InstructionRes>(5, ArithmeticTarget::A, 8);
  } else if (opcode == 0xB0) {  // RES 6, B
    EMIT_INSTRUCTION(pc_begin, "RES 6, B");
    return std::make_unique<InstructionRes>(6, ArithmeticTarget::B, 8);
  } else if (opcode == 0xB1) {  // RES 6, C
    EMIT_INSTRUCTION(pc_begin, "RES 6, C");
    return std::make_unique<InstructionRes>(6, ArithmeticTarget::C, 8);
  } else if (opcode == 0xB2) {  // RES 6, D
    EMIT_INSTRUCTION(pc_begin, "RES 6, D");
    return std::make_unique<InstructionRes>(6, ArithmeticTarget::D, 8);
  } else if (opcode == 0xB3) {  // RES 6, E
    EMIT_INSTRUCTION(pc_begin, "RES 6, E");
    return std::make_unique<InstructionRes>(6, ArithmeticTarget::E, 8);
  } else if (opcode == 0xB4) {  // RES 6, H
    EMIT_INSTRUCTION(pc_begin, "RES 6, H");
    return std::make_unique<InstructionRes>(6, ArithmeticTarget::H, 8);
  } else if (opcode == 0xB5) {  // RES 6, L
    EMIT_INSTRUCTION(pc_begin, "RES 6, L");
    return std::make_unique<InstructionRes>(6, ArithmeticTarget::L, 8);
  } else if (opcode == 0xB6) {  // RES 6, [HL]
    EMIT_INSTRUCTION(pc_begin, "RES 6, [HL]");
    return std::make_unique<InstructionRes>(6, ArithmeticTarget::HL, true, 16);
  } else if (opcode == 0xB7) {  // RES 6, A
    EMIT_INSTRUCTION(pc_begin, "RES 6, A");
    return std::make_unique<InstructionRes>(6, ArithmeticTarget::A, 8);
  } else if (opcode == 0xB8) {  // RES 7, B
    EMIT_INSTRUCTION(pc_begin, "RES 7, B");
    return std::make_unique<InstructionRes>(7, ArithmeticTarget::B, 8);
  } else if (opcode == 0xB9) {  // RES 7, C
    EMIT_INSTRUCTION(pc_begin, "RES 7, C");
    return std::make_unique<InstructionRes>(7, ArithmeticTarget::C, 8);
  } else if (opcode == 0xBA) {  // RES 7, D
    EMIT_INSTRUCTION(pc_begin, "RES 7, D");
    return std::make_unique<InstructionRes>(7, ArithmeticTarget::D, 8);
  } else if (opcode == 0xBB) {  // RES 7, E
    EMIT_INSTRUCTION(pc_begin, "RES 7, E");
    return std::make_unique<InstructionRes>(7, ArithmeticTarget::E, 8);
  } else if (opcode == 0xBC) {  // RES 7, H
    EMIT_INSTRUCTION(pc_begin, "RES 7, H");
    return std::make_unique<InstructionRes>(7, ArithmeticTarget::H, 8);
  } else if (opcode == 0xBD) {  // RES 7, L
    EMIT_INSTRUCTION(pc_begin, "RES 7, L");
    return std::make_unique<InstructionRes>(7, ArithmeticTarget::L, 8);
  } else if (opcode == 0xBE) {  // RES 7, [HL]
    EMIT_INSTRUCTION(pc_begin, "RES 7, [HL]");
    return std::make_unique<InstructionRes>(7, ArithmeticTarget::HL, true, 16);
  } else if (opcode == 0xBF) {  // RES 7, A
    EMIT_INSTRUCTION(pc_begin, "RES 7, A");
    return std::make_unique<InstructionRes>(7, ArithmeticTarget::A, 8);
  } else if (opcode == 0xC0) {  // SET 0, B
    EMIT_INSTRUCTION(pc_begin, "SET 0, B");
    return std::make_unique<InstructionSet>(0, ArithmeticTarget::B, 8);
  } else if (opcode == 0xC1) {  // SET 0, C
    EMIT_INSTRUCTION(pc_begin, "SET 0, C");
    return std::make_unique<InstructionSet>(0, ArithmeticTarget::C, 8);
  } else if (opcode == 0xC2) {  // SET 0, D
    EMIT_INSTRUCTION(pc_begin, "SET 0, D");
    return std::make_unique<InstructionSet>(0, ArithmeticTarget::D, 8);
  } else if (opcode == 0xC3) {  // SET 0, E
    EMIT_INSTRUCTION(pc_begin, "SET 0, E");
    return std::make_unique<InstructionSet>(0, ArithmeticTarget::E, 8);
  } else if (opcode == 0xC4) {  // SET 0, H
    EMIT_INSTRUCTION(pc_begin, "SET 0, H");
    return std::make_unique<InstructionSet>(0, ArithmeticTarget::H, 8);
  } else if (opcode == 0xC5) {  // SET 0, L
    EMIT_INSTRUCTION(pc_begin, "SET 0, L");
    return std::make_unique<InstructionSet>(0, ArithmeticTarget::L, 8);
  } else if (opcode == 0xC6) {  // SET 0, [HL]
    EMIT_INSTRUCTION(pc_begin, "SET 0, [HL]");
    return std::make_unique<InstructionSet>(0, ArithmeticTarget::HL, true, 16);
  } else if (opcode == 0xC7) {  // SET 0, A
    EMIT_INSTRUCTION(pc_begin, "SET 0, A");
    return std::make_unique<InstructionSet>(0, ArithmeticTarget::A, 8);
  } else if (opcode == 0xC8) {  // SET 1, B
    EMIT_INSTRUCTION(pc_begin, "SET 1, B");
    return std::make_unique<InstructionSet>(1, ArithmeticTarget::B, 8);
  } else if (opcode == 0xC9) {  // SET 1, C
    EMIT_INSTRUCTION(pc_begin, "SET 1, C");
    return std::make_unique<InstructionSet>(1, ArithmeticTarget::C, 8);
  } else if (opcode == 0xCA) {  // SET 1, D
    EMIT_INSTRUCTION(pc_begin, "SET 1, D");
    return std::make_unique<InstructionSet>(1, ArithmeticTarget::D, 8);
  } else if (opcode == 0xCB) {  // SET 1, E
    EMIT_INSTRUCTION(pc_begin, "SET 1, E");
    return std::make_unique<InstructionSet>(1, ArithmeticTarget::E, 8);
  } else if (opcode == 0xCC) {  // SET 1, H
    EMIT_INSTRUCTION(pc_begin, "SET 1, H");
    return std::make_unique<InstructionSet>(1, ArithmeticTarget::H, 8);
  } else if (opcode == 0xCD) {  // SET 1, L
    EMIT_INSTRUCTION(pc_begin, "SET 1, L");
    return std::make_unique<InstructionSet>(1, ArithmeticTarget::L, 8);
  } else if (opcode == 0xCE) {  // SET 1, [HL]
    EMIT_INSTRUCTION(pc_begin, "SET 1, [HL]");
    return std::make_unique<InstructionSet>(1, ArithmeticTarget::HL, true, 16);
  } else if (opcode == 0xCF) {  // SET 1, A
    EMIT_INSTRUCTION(pc_begin, "SET 1, A");
    return std::make_unique<InstructionSet>(1, ArithmeticTarget::A, 8);
  } else if (opcode == 0xD0) {  // SET 2, B
    EMIT_INSTRUCTION(pc_begin, "SET 2, B");
    return std::make_unique<InstructionSet>(2, ArithmeticTarget::B, 8);
  } else if (opcode == 0xD1) {  // SET 2, C
    EMIT_INSTRUCTION(pc_begin, "SET 2, C");
    return std::make_unique<InstructionSet>(2, ArithmeticTarget::C, 8);
  } else if (opcode == 0xD2) {  // SET 2, D
    EMIT_INSTRUCTION(pc_begin, "SET 2, D");
    return std::make_unique<InstructionSet>(2, ArithmeticTarget::D, 8);
  } else if (opcode == 0xD3) {  // SET 2, E
    EMIT_INSTRUCTION(pc_begin, "SET 2, E");
    return std::make_unique<InstructionSet>(2, ArithmeticTarget::E, 8);
  } else if (opcode == 0xD4) {  // SET 2, H
    EMIT_INSTRUCTION(pc_begin, "SET 2, H");
    return std::make_unique<InstructionSet>(2, ArithmeticTarget::H, 8);
  } else if (opcode == 0xD5) {  // SET 2, L
    EMIT_INSTRUCTION(pc_begin, "SET 2, L");
    return std::make_unique<InstructionSet>(2, ArithmeticTarget::L, 8);
  } else if (opcode == 0xD6) {  // SET 2, [HL]
    EMIT_INSTRUCTION(pc_begin, "SET 2, [HL]");
    return std::make_unique<InstructionSet>(2, ArithmeticTarget::HL, true, 16);
  } else if (opcode == 0xD7) {  // SET 2, A
    EMIT_INSTRUCTION(pc_begin, "SET 2, A");
    return std::make_unique<InstructionSet>(2, ArithmeticTarget::A, 8);
  } else if (opcode == 0xD8) {  // SET 3, B
    EMIT_INSTRUCTION(pc_begin, "SET 3, B");
    return std::make_unique<InstructionSet>(3, ArithmeticTarget::B, 8);
  } else if (opcode == 0xD9) {  // SET 3, C
    EMIT_INSTRUCTION(pc_begin, "SET 3, C");
    return std::make_unique<InstructionSet>(3, ArithmeticTarget::C, 8);
  } else if (opcode == 0xDA) {  // SET 3, D
    EMIT_INSTRUCTION(pc_begin, "SET 3, D");
    return std::make_unique<InstructionSet>(3, ArithmeticTarget::D, 8);
  } else if (opcode == 0xDB) {  // SET 3, E
    EMIT_INSTRUCTION(pc_begin, "SET 3, E");
    return std::make_unique<InstructionSet>(3, ArithmeticTarget::E, 8);
  } else if (opcode == 0xDC) {  // SET 3, H
    EMIT_INSTRUCTION(pc_begin, "SET 3, H");
    return std::make_unique<InstructionSet>(3, ArithmeticTarget::H, 8);
  } else if (opcode == 0xDD) {  // SET 3, L
    EMIT_INSTRUCTION(pc_begin, "SET 3, L");
    return std::make_unique<InstructionSet>(3, ArithmeticTarget::L, 8);
  } else if (opcode == 0xDE) {  // SET 3, [HL]
    EMIT_INSTRUCTION(pc_begin, "SET 3, [HL]");
    return std::make_unique<InstructionSet>(3, ArithmeticTarget::HL, true, 16);
  } else if (opcode == 0xDF) {  // SET 3, A
    EMIT_INSTRUCTION(pc_begin, "SET 3, A");
    return std::make_unique<InstructionSet>(3, ArithmeticTarget::A, 8);
  } else if (opcode == 0xE0) {  // SET 4, B
    EMIT_INSTRUCTION(pc_begin, "SET 4, B");
    return std::make_unique<InstructionSet>(4, ArithmeticTarget::B, 8);
  } else if (opcode == 0xE1) {  // SET 4, C
    EMIT_INSTRUCTION(pc_begin, "SET 4, C");
    return std::make_unique<InstructionSet>(4, ArithmeticTarget::C, 8);
  } else if (opcode == 0xE2) {  // SET 4, D
    EMIT_INSTRUCTION(pc_begin, "SET 4, D");
    return std::make_unique<InstructionSet>(4, ArithmeticTarget::D, 8);
  } else if (opcode == 0xE3) {  // SET 4, E
    EMIT_INSTRUCTION(pc_begin, "SET 4, E");
    return std::make_unique<InstructionSet>(4, ArithmeticTarget::E, 8);
  } else if (opcode == 0xE4) {  // SET 4, H
    EMIT_INSTRUCTION(pc_begin, "SET 4, H");
    return std::make_unique<InstructionSet>(4, ArithmeticTarget::H, 8);
  } else if (opcode == 0xE5) {  // SET 4, L
    EMIT_INSTRUCTION(pc_begin, "SET 4, L");
    return std::make_unique<InstructionSet>(4, ArithmeticTarget::L, 8);
  } else if (opcode == 0xE6) {  // SET 4, [HL]
    EMIT_INSTRUCTION(pc_begin, "SET 4, [HL]");
    return std::make_unique<InstructionSet>(4, ArithmeticTarget::HL, true, 16);
  } else if (opcode == 0xE7) {  // SET 4, A
    EMIT_INSTRUCTION(pc_begin, "SET 4, A");
    return std::make_unique<InstructionSet>(4, ArithmeticTarget::A, 8);
  } else if (opcode == 0xE8) {  // SET 5, B
    EMIT_INSTRUCTION(pc_begin, "SET 5, B");
    return std::make_unique<InstructionSet>(5, ArithmeticTarget::B, 8);
  } else if (opcode == 0xE9) {  // SET 5, C
    EMIT_INSTRUCTION(pc_begin, "SET 5, C");
    return std::make_unique<InstructionSet>(5, ArithmeticTarget::C, 8);
  } else if (opcode == 0xEA) {  // SET 5, D
    EMIT_INSTRUCTION(pc_begin, "SET 5, D");
    return std::make_unique<InstructionSet>(5, ArithmeticTarget::D, 8);
  } else if (opcode == 0xEB) {  // SET 5, E
    EMIT_INSTRUCTION(pc_begin, "SET 5, E");
    return std::make_unique<InstructionSet>(5, ArithmeticTarget::E, 8);
  } else if (opcode == 0xEC) {  // SET 5, H
    EMIT_INSTRUCTION(pc_begin, "SET 5, H");
    return std::make_unique<InstructionSet>(5, ArithmeticTarget::H, 8);
  } else if (opcode == 0xED) {  // SET 5, L
    EMIT_INSTRUCTION(pc_begin, "SET 5, L");
    return std::make_unique<InstructionSet>(5, ArithmeticTarget::L, 8);
  } else if (opcode == 0xEE) {  // SET 5, [HL]
    EMIT_INSTRUCTION(pc_begin, "SET 5, [HL]");
    return std::make_unique<InstructionSet>(5, ArithmeticTarget::HL, true, 16);
  } else if (opcode == 0xEF) {  // SET 5, A
    EMIT_INSTRUCTION(pc_begin, "SET 5, A");
    return std::make_unique<InstructionSet>(5, ArithmeticTarget::A, 8);
  } else if (opcode == 0xF0) {  // SET 6, B
    EMIT_INSTRUCTION(pc_begin, "SET 6, B");
    return std::make_unique<InstructionSet>(6, ArithmeticTarget::B, 8);
  } else if (opcode == 0xF1) {  // SET 6, C
    EMIT_INSTRUCTION(pc_begin, "SET 6, C");
    return std::make_unique<InstructionSet>(6, ArithmeticTarget::C, 8);
  } else if (opcode == 0xF2) {  // SET 6, D
    EMIT_INSTRUCTION(pc_begin, "SET 6, D");
    return std::make_unique<InstructionSet>(6, ArithmeticTarget::D, 8);
  } else if (opcode == 0xF3) {  // SET 6, E
    EMIT_INSTRUCTION(pc_begin, "SET 6, E");
    return std::make_unique<InstructionSet>(6, ArithmeticTarget::E, 8);
  } else if (opcode == 0xF4) {  // SET 6, H
    EMIT_INSTRUCTION(pc_begin, "SET 6, H");
    return std::make_unique<InstructionSet>(6, ArithmeticTarget::H, 8);
  } else if (opcode == 0xF5) {  // SET 6, L
    EMIT_INSTRUCTION(pc_begin, "SET 6, L");
    return std::make_unique<InstructionSet>(6, ArithmeticTarget::L, 8);
  } else if (opcode == 0xF6) {  // SET 6, [HL]
    EMIT_INSTRUCTION(pc_begin, "SET 6, [HL]");
    return std::make_unique<InstructionSet>(6, ArithmeticTarget::HL, true, 16);
  } else if (opcode == 0xF7) {  // SET 6, A
    EMIT_INSTRUCTION(pc_begin, "SET 6, A");
    return std::make_unique<InstructionSet>(6, ArithmeticTarget::A, 8);
  } else if (opcode == 0xF8) {  // SET 7, B
    EMIT_INSTRUCTION(pc_begin, "SET 7, B");
    return std::make_unique<InstructionSet>(7, ArithmeticTarget::B, 8);
  } else if (opcode == 0xF9) {  // SET 7, C
    EMIT_INSTRUCTION(pc_begin, "SET 7, C");
    return std::make_unique<InstructionSet>(7, ArithmeticTarget::C, 8);
  } else if (opcode == 0xFA) {  // SET 7, D
    EMIT_INSTRUCTION(pc_begin, "SET 7, D");
    return std::make_unique<InstructionSet>(7, ArithmeticTarget::D, 8);
  } else if (opcode == 0xFB) {  // SET 7, E
    EMIT_INSTRUCTION(pc_begin, "SET 7, E");
    return std::make_unique<InstructionSet>(7, ArithmeticTarget::E, 8);
  } else if (opcode == 0xFC) {  // SET 7, H
    EMIT_INSTRUCTION(pc_begin, "SET 7, H");
    return std::make_unique<InstructionSet>(7, ArithmeticTarget::H, 8);
  } else if (opcode == 0xFD) {  // SET 7, L
    EMIT_INSTRUCTION(pc_begin, "SET 7, L");
    return std::make_unique<InstructionSet>(7, ArithmeticTarget::L, 8);
  } else if (opcode == 0xFE) {  // SET 7, [HL]
    EMIT_INSTRUCTION(pc_begin, "SET 7, [HL]");
    return std::make_unique<InstructionSet>(7, ArithmeticTarget::HL, true, 16);
  } else if (opcode == 0xFF) {  // SET 7, A
    EMIT_INSTRUCTION(pc_begin, "SET 7, A");
    return std::make_unique<InstructionSet>(7, ArithmeticTarget::A, 8);
  }
  //std::cout << "unknown instruction ($CB): " << ToHex(opcode) << std::endl;
  return nullptr;
}

std::unique_ptr<Instruction> Fetch(ALU& alu, Registers& registers, MemoryBus& bus) {
  u16 pc_begin = registers.pc;
  u8 opcode = Fetch(registers, bus);
  if (opcode == 0x00) {  // NOP
    EMIT_INSTRUCTION(pc_begin, "NOP");
    return std::make_unique<InstructionNoOp>();
  } else if (opcode == 0x01) {  // LD BC, n16
    u16 value = FetchWord(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "LD BC, {}", ToHex(value));
    return std::make_unique<InstructionLoadImmediate>(ArithmeticTarget::BC, false, value, 12);
  } else if (opcode == 0x02) {  // LD [BC], A
    EMIT_INSTRUCTION(pc_begin, "LD [BC], A");
    return std::make_unique<InstructionLoad>(ArithmeticTarget::BC, LoadOperandType::AS_ADDRESS, ArithmeticTarget::A, LoadOperandType::REGISTER, 8);
  } else if (opcode == 0x03) {  // INC BC
    EMIT_INSTRUCTION(pc_begin, "INC BC");
    return std::make_unique<InstructionInc>(ArithmeticTarget::BC, 8);
  } else if (opcode == 0x04) {  // INC B
    EMIT_INSTRUCTION(pc_begin, "INC B");
    return std::make_unique<InstructionInc>(ArithmeticTarget::B, 4);
  } else if (opcode == 0x05) {  // DEC B
    EMIT_INSTRUCTION(pc_begin, "DEC B");
    return std::make_unique<InstructionDec>(ArithmeticTarget::B, 4);
  } else if (opcode == 0x06) {  // LD B, n8
    u8 value = Fetch(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "LD B, {}", ToHex(value));
    return std::make_unique<InstructionLoadImmediate>(ArithmeticTarget::B, false, value, 8);
  } else if (opcode == 0x07) {  // RCLA
    EMIT_INSTRUCTION(pc_begin, "RCLA");
    return std::make_unique<InstructionRotateLeftCircular>();
  } else if (opcode == 0x08) {  // LD [a16], SP
    u16 value = FetchWord(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "LD [{}], SP", ToHex(value));
    return std::make_unique<InstructionLoadToAddress>(value, ArithmeticTarget::SP, 20);
  } else if (opcode == 0x09) {  // ADD HL, BC
    EMIT_INSTRUCTION(pc_begin, "ADD HL, BC");
    return std::make_unique<InstructionAdd>(ArithmeticTarget::HL, ArithmeticTarget::BC, false, 8);
  } else if (opcode == 0x0A) {  // LD A, [BC]
    EMIT_INSTRUCTION(pc_begin, "LD A, [BC]");
    return std::make_unique<InstructionLoad>(ArithmeticTarget::A, LoadOperandType::REGISTER, ArithmeticTarget::BC, LoadOperandType::AS_ADDRESS, 8);
  } else if (opcode == 0x0B) {  // DEC BC
    EMIT_INSTRUCTION(pc_begin, "DEC BC");
    return std::make_unique<InstructionDec>(ArithmeticTarget::BC, 8);
  } else if (opcode == 0x0C) {  // INC C
    EMIT_INSTRUCTION(pc_begin, "INC C");
    return std::make_unique<InstructionInc>(ArithmeticTarget::C, 4);
  } else if (opcode == 0x0D) {  // DEC C
    EMIT_INSTRUCTION(pc_begin, "DEC C");
    return std::make_unique<InstructionDec>(ArithmeticTarget::C, 4);
  } else if (opcode == 0x0E) {  // LD C, n8
    u8 value = Fetch(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "LD C, [{}]", ToHex(value));
    return std::make_unique<InstructionLoadImmediate>(ArithmeticTarget::C, false, value, 8);
  } else if (opcode == 0x0F) {  // RRCA
    EMIT_INSTRUCTION(pc_begin, "RRCA");
    return std::make_unique<InstructionRotateRightCircular>();
  } else if (opcode == 0x10) {  // STOP
    //Fetch(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "STOP");
    return std::make_unique<InstructionStop>();
  } else if (opcode == 0x11) {  // LD DE, n16
    u16 value = FetchWord(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "LD DE, {}", ToHex(value));
    return std::make_unique<InstructionLoadImmediate>(ArithmeticTarget::DE, false, value, 12);
  } else if (opcode == 0x12) {  // LD [DE], a
    EMIT_INSTRUCTION(pc_begin, "LD [DE], a");
    return std::make_unique<InstructionLoad>(ArithmeticTarget::DE, LoadOperandType::AS_ADDRESS, ArithmeticTarget::A, LoadOperandType::REGISTER, 8);
  } else if (opcode == 0x13) {  // INC DE
    EMIT_INSTRUCTION(pc_begin, "INC DE");
    return std::make_unique<InstructionInc>(ArithmeticTarget::DE, 8);
  } else if (opcode == 0x14) {  // INC D
    EMIT_INSTRUCTION(pc_begin, "INC D");
    return std::make_unique<InstructionInc>(ArithmeticTarget::D, 4);
  } else if (opcode == 0x15) {  // DEC D
    EMIT_INSTRUCTION(pc_begin, "DEC D");
    return std::make_unique<InstructionDec>(ArithmeticTarget::D, 4);
  } else if (opcode == 0x16) {  // LD D, n8
    u8 value = Fetch(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "LD D, {}", ToHex(value));
    return std::make_unique<InstructionLoadImmediate>(ArithmeticTarget::D, false, value, 8);
  } else if (opcode == 0x17) {  // RLA
    EMIT_INSTRUCTION(pc_begin, "RLA");
    return std::make_unique<InstructionRotateLeft>();
  } else if (opcode == 0x18) {  // JR e8
    s8 value = AsSigned(Fetch(registers, bus));
    EMIT_INSTRUCTION(pc_begin, "JR {}", ToHex(value));
    return std::make_unique<InstructionJumpRelative>(value);
  } else if (opcode == 0x19) {  // ADD HL, DE
    EMIT_INSTRUCTION(pc_begin, "ADD HL, DE");
    return std::make_unique<InstructionAdd>(ArithmeticTarget::HL, ArithmeticTarget::DE, false, 8);
  } else if (opcode == 0x1A) {  // LD A, [DE]
    EMIT_INSTRUCTION(pc_begin, "LD A, [DE]");
    return std::make_unique<InstructionLoad>(ArithmeticTarget::A, LoadOperandType::REGISTER, ArithmeticTarget::DE, LoadOperandType::AS_ADDRESS, 8);
  } else if (opcode == 0x1B) {  // DEC DE
    EMIT_INSTRUCTION(pc_begin, "DEC DE");
    return std::make_unique<InstructionDec>(ArithmeticTarget::DE, 8);
  } else if (opcode == 0x1C) {  // INC E
    EMIT_INSTRUCTION(pc_begin, "INC E");
    return std::make_unique<InstructionInc>(ArithmeticTarget::E, 4);
  } else if (opcode == 0x1D) {  // DEC E
    EMIT_INSTRUCTION(pc_begin, "DEC E");
    return std::make_unique<InstructionDec>(ArithmeticTarget::E, 4);
  } else if (opcode == 0x1E) {  // LD E, n8
    u8 value = Fetch(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "LD E, {}", ToHex(value));
    return std::make_unique<InstructionLoadImmediate>(ArithmeticTarget::E, false, value, 8);
  } else if (opcode == 0x1F) {  // RRA
    EMIT_INSTRUCTION(pc_begin, "RRA");
    return std::make_unique<InstructionRotateRight>();
  } else if (opcode == 0x20) {  // JR NZ, e8
    s8 value = AsSigned(Fetch(registers, bus));
    EMIT_INSTRUCTION(pc_begin, "JR NZ, {}", ToHex(value));
    return std::make_unique<InstructionJumpRelativeIfZero>(value, true);
  } else if (opcode == 0x21) {  // LD HL, n16
    u16 value = FetchWord(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "LD HL, {}", ToHex(value));
    return std::make_unique<InstructionLoadImmediate>(ArithmeticTarget::HL, false, value, 12);
  } else if (opcode == 0x22) {  // LD [HL+], A
    EMIT_INSTRUCTION(pc_begin, "LD [HL+], A");
    return std::make_unique<InstructionLoad>(ArithmeticTarget::HL, LoadOperandType::AS_ADDRESS_INC, ArithmeticTarget::A, LoadOperandType::REGISTER, 8);
  } else if (opcode == 0x23) {  // INC HL
    EMIT_INSTRUCTION(pc_begin, "INC HL");
    return std::make_unique<InstructionInc>(ArithmeticTarget::HL, 8);
  } else if (opcode == 0x24) {  // INC H
    EMIT_INSTRUCTION(pc_begin, "INC H");
    return std::make_unique<InstructionInc>(ArithmeticTarget::H, 4);
  } else if (opcode == 0x25) {  // DEC H
    EMIT_INSTRUCTION(pc_begin, "DEC H");
    return std::make_unique<InstructionDec>(ArithmeticTarget::H, 4);
  } else if (opcode == 0x26) {  // LD H, n8
    u8 value = Fetch(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "LD H, {}", ToHex(value));
    return std::make_unique<InstructionLoadImmediate>(ArithmeticTarget::H, false, value, 8);
  } else if (opcode == 0x27) {  // DAA
    EMIT_INSTRUCTION(pc_begin, "DAA");
    return std::make_unique<InstructionDAA>();
  } else if (opcode == 0x28) {  // JR Z, e8
    s8 value = AsSigned(Fetch(registers, bus));
    EMIT_INSTRUCTION(pc_begin, "JR Z, {}", ToHex(value));
    return std::make_unique<InstructionJumpRelativeIfZero>(value, false);
  } else if (opcode == 0x29) {  // ADD HL, HL
    EMIT_INSTRUCTION(pc_begin, "ADD HL, HL");
    return std::make_unique<InstructionAdd>(ArithmeticTarget::HL, ArithmeticTarget::HL, false, 8);
  } else if (opcode == 0x2A) {  // LD A, [HL+]
    EMIT_INSTRUCTION(pc_begin, "LD A, [HL+]");
    return std::make_unique<InstructionLoad>(ArithmeticTarget::A, LoadOperandType::REGISTER, ArithmeticTarget::HL, LoadOperandType::AS_ADDRESS_INC, 8);
  } else if (opcode == 0x2B) {  // DEC HL
    EMIT_INSTRUCTION(pc_begin, "DEC HL");
    return std::make_unique<InstructionDec>(ArithmeticTarget::HL, 8);
  } else if (opcode == 0x2C) {  // INC L
    EMIT_INSTRUCTION(pc_begin, "INC L");
    return std::make_unique<InstructionInc>(ArithmeticTarget::L, 4);
  } else if (opcode == 0x2D) {  // DEC L
    EMIT_INSTRUCTION(pc_begin, "DEC L");
    return std::make_unique<InstructionDec>(ArithmeticTarget::L, 4);
  } else if (opcode == 0x2E) {  // LD L, n8
    u8 value = Fetch(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "LD L, {}", ToHex(value));
    return std::make_unique<InstructionLoadImmediate>(ArithmeticTarget::L, false, value, 8);
  } else if (opcode == 0x2F) {  // CPL
    EMIT_INSTRUCTION(pc_begin, "CPL");
    return std::make_unique<InstructionComplement>();
  } else if (opcode == 0x30) {  // JR NC, e8
    s8 value = AsSigned(Fetch(registers, bus));
    EMIT_INSTRUCTION(pc_begin, "JR NC, {}", ToHex(value));
    return std::make_unique<InstructionJumpRelativeIfCarry>(value, true);
  } else if (opcode == 0x31) {  // LD SP, n16
    u16 value = FetchWord(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "LD SP, {}", ToHex(value));
    return std::make_unique<InstructionLoadImmediate>(ArithmeticTarget::SP, false, value, 12);
  } else if (opcode == 0x32) {  // LD [HL-], A
    EMIT_INSTRUCTION(pc_begin, "LD [HL-], A");
    return std::make_unique<InstructionLoad>(ArithmeticTarget::HL, LoadOperandType::AS_ADDRESS_DEC, ArithmeticTarget::A, LoadOperandType::REGISTER, 8);
  } else if (opcode == 0x33) {  // INC SP
    EMIT_INSTRUCTION(pc_begin, "INC SP");
    return std::make_unique<InstructionInc>(ArithmeticTarget::SP, 8);
  } else if (opcode == 0x34) {  // INC [HL]
    EMIT_INSTRUCTION(pc_begin, "INC [HL]");
    return std::make_unique<InstructionInc>(ArithmeticTarget::HL, IncDecOperandType::MEMORY, 12);
  } else if (opcode == 0x35) {  // DEC [HL]
    EMIT_INSTRUCTION(pc_begin, "DEC [HL]");
    return std::make_unique<InstructionDec>(ArithmeticTarget::HL, IncDecOperandType::MEMORY, 12);
  } else if (opcode == 0x36) {  // LD [HL], n8
    u8 value = Fetch(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "LD [HL], {}", ToHex(value));
    return std::make_unique<InstructionLoadImmediate>(ArithmeticTarget::HL, true, value, 12);
  } else if (opcode == 0x37) {  // SCF
    EMIT_INSTRUCTION(pc_begin, "SCF");
    return std::make_unique<InstructionSetCarryFlag>();
  } else if (opcode == 0x38) {  // JR C, e8
    s8 value = AsSigned(Fetch(registers, bus));
    EMIT_INSTRUCTION(pc_begin, "JR C, {}", ToHex(value));
    return std::make_unique<InstructionJumpRelativeIfCarry>(value, false);
  } else if (opcode == 0x39) {  // ADD HL, SP
    EMIT_INSTRUCTION(pc_begin, "ADD HL, SP");
    return std::make_unique<InstructionAdd>(ArithmeticTarget::HL, ArithmeticTarget::SP, false, 8);
  } else if (opcode == 0x3A) {  // LD A, [HL-]
    EMIT_INSTRUCTION(pc_begin, "LD A, [HL-]");
    return std::make_unique<InstructionLoad>(ArithmeticTarget::A, LoadOperandType::REGISTER, ArithmeticTarget::HL, LoadOperandType::AS_ADDRESS_DEC, 8);
  } else if (opcode == 0x3B) {  // DEC SP
    EMIT_INSTRUCTION(pc_begin, "DEC SP");
    return std::make_unique<InstructionDec>(ArithmeticTarget::SP, 8);
  } else if (opcode == 0x3C) {  // INC A
    EMIT_INSTRUCTION(pc_begin, "INC A");
    return std::make_unique<InstructionInc>(ArithmeticTarget::A, 4);
  } else if (opcode == 0x3D) {  // DEC A
    EMIT_INSTRUCTION(pc_begin, "DEC A");
    return std::make_unique<InstructionDec>(ArithmeticTarget::A, 4);
  } else if (opcode == 0x3E) {  // LD A, n8
    u8 value = Fetch(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "LD A, {}", ToHex(value));
    return std::make_unique<InstructionLoadImmediate>(ArithmeticTarget::A, false, value, 8);
  } else if (opcode == 0x3F) {  // CCF
    EMIT_INSTRUCTION(pc_begin, "CCF");
    return std::make_unique<InstructionComplementCarryFlag>();
  } else if (opcode == 0x40) {  // LD B, B
    EMIT_INSTRUCTION(pc_begin, "LD B, B");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::B, ArithmeticTarget::B, 4);
  } else if (opcode == 0x41) {  // LD B, C
    EMIT_INSTRUCTION(pc_begin, "LD B, C");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::B, ArithmeticTarget::C, 4);
  } else if (opcode == 0x42) {  // LD B, D
    EMIT_INSTRUCTION(pc_begin, "LD B, D");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::B, ArithmeticTarget::D, 4);
  } else if (opcode == 0x43) {  // LD B, E
    EMIT_INSTRUCTION(pc_begin, "LD B, E");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::B, ArithmeticTarget::E, 4);
  } else if (opcode == 0x44) {  // LD B, H
    EMIT_INSTRUCTION(pc_begin, "LD B, H");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::B, ArithmeticTarget::H, 4);
  } else if (opcode == 0x45) {  // LD B, L
    EMIT_INSTRUCTION(pc_begin, "LD B, L");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::B, ArithmeticTarget::L, 4);
  } else if (opcode == 0x46) {  // LD B, [HL]
    EMIT_INSTRUCTION(pc_begin, "LD B, [HL]");
    return std::make_unique<InstructionLoad>(ArithmeticTarget::B, LoadOperandType::REGISTER, ArithmeticTarget::HL, LoadOperandType::AS_ADDRESS, 8);
  } else if (opcode == 0x47) {  // LD B, A
    EMIT_INSTRUCTION(pc_begin, "LD B, A");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::B, ArithmeticTarget::A, 4);
  } else if (opcode == 0x48) {  // LD C, B
    EMIT_INSTRUCTION(pc_begin, "LD C, B");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::C, ArithmeticTarget::B, 4);
  } else if (opcode == 0x49) {  // LD C, C
    EMIT_INSTRUCTION(pc_begin, "LD C, C");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::C, ArithmeticTarget::C, 4);
  } else if (opcode == 0x4A) {  // LD C, D
    EMIT_INSTRUCTION(pc_begin, "LD C, D");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::C, ArithmeticTarget::D, 4);
  } else if (opcode == 0x4B) {  // LD C, E
    EMIT_INSTRUCTION(pc_begin, "LD C, E");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::C, ArithmeticTarget::E, 4);
  } else if (opcode == 0x4C) {  // LD C, H
    EMIT_INSTRUCTION(pc_begin, "LD C, H");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::C, ArithmeticTarget::H, 4);
  } else if (opcode == 0x4D) {  // LD C, L
    EMIT_INSTRUCTION(pc_begin, "LD C, L");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::C, ArithmeticTarget::L, 4);
  } else if (opcode == 0x4E) {  // LD C, [HL]
    EMIT_INSTRUCTION(pc_begin, "LD C, [HL]");
    return std::make_unique<InstructionLoad>(ArithmeticTarget::C, LoadOperandType::REGISTER, ArithmeticTarget::HL, LoadOperandType::AS_ADDRESS, 8);
  } else if (opcode == 0x4F) {  // LD C, A
    EMIT_INSTRUCTION(pc_begin, "LD C, A");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::C, ArithmeticTarget::A, 4);
  } else if (opcode == 0x50) {  // LD D, B
    EMIT_INSTRUCTION(pc_begin, "LD D, B");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::D, ArithmeticTarget::B, 4);
  } else if (opcode == 0x51) {  // LD D, C
    EMIT_INSTRUCTION(pc_begin, "LD D, C");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::D, ArithmeticTarget::C, 4);
  } else if (opcode == 0x52) {  // LD D, D
    EMIT_INSTRUCTION(pc_begin, "LD D, D");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::D, ArithmeticTarget::D, 4);
  } else if (opcode == 0x53) {  // LD D, E
    EMIT_INSTRUCTION(pc_begin, "LD D, E");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::D, ArithmeticTarget::E, 4);
  } else if (opcode == 0x54) {  // LD D, H
    EMIT_INSTRUCTION(pc_begin, "LD D, H");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::D, ArithmeticTarget::H, 4);
  } else if (opcode == 0x55) {  // LD D, L
    EMIT_INSTRUCTION(pc_begin, "LD D, L");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::D, ArithmeticTarget::L, 4);
  } else if (opcode == 0x56) {  // LD D, [HL]
    EMIT_INSTRUCTION(pc_begin, "LD D, [HL]");
    return std::make_unique<InstructionLoad>(ArithmeticTarget::D, LoadOperandType::REGISTER, ArithmeticTarget::HL, LoadOperandType::AS_ADDRESS, 8);
  } else if (opcode == 0x57) {  // LD D, A
    EMIT_INSTRUCTION(pc_begin, "LD D, A");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::D, ArithmeticTarget::A, 4);
  } else if (opcode == 0x58) {  // LD E, B
    EMIT_INSTRUCTION(pc_begin, "LD E, B");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::E, ArithmeticTarget::B, 4);
  } else if (opcode == 0x59) {  // LD E, C
    EMIT_INSTRUCTION(pc_begin, "LD E, C");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::E, ArithmeticTarget::C, 4);
  } else if (opcode == 0x5A) {  // LD E, D
    EMIT_INSTRUCTION(pc_begin, "LD E, D");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::E, ArithmeticTarget::D, 4);
  } else if (opcode == 0x5B) {  // LD E, E
    EMIT_INSTRUCTION(pc_begin, "LD E, E");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::E, ArithmeticTarget::E, 4);
  } else if (opcode == 0x5C) {  // LD E, H
    EMIT_INSTRUCTION(pc_begin, "LD E, H");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::E, ArithmeticTarget::H, 4);
  } else if (opcode == 0x5D) {  // LD E, L
    EMIT_INSTRUCTION(pc_begin, "LD E, L");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::E, ArithmeticTarget::L, 4);
  } else if (opcode == 0x5E) {  // LD E, [HL]
    EMIT_INSTRUCTION(pc_begin, "LD E, [HL]");
    return std::make_unique<InstructionLoad>(ArithmeticTarget::E, LoadOperandType::REGISTER, ArithmeticTarget::HL, LoadOperandType::AS_ADDRESS, 8);
  } else if (opcode == 0x5F) {  // LD E, A
    EMIT_INSTRUCTION(pc_begin, "LD E, A");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::E, ArithmeticTarget::A, 4);
  } else if (opcode == 0x60) {  // LD H, B
    EMIT_INSTRUCTION(pc_begin, "LD H, B");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::H, ArithmeticTarget::B, 4);
  } else if (opcode == 0x61) {  // LD H, C
    EMIT_INSTRUCTION(pc_begin, "LD H, C");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::H, ArithmeticTarget::C, 4);
  } else if (opcode == 0x62) {  // LD H, D
    EMIT_INSTRUCTION(pc_begin, "LD H, D");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::H, ArithmeticTarget::D, 4);
  } else if (opcode == 0x63) {  // LD H, E
    EMIT_INSTRUCTION(pc_begin, "LD H, E");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::H, ArithmeticTarget::E, 4);
  } else if (opcode == 0x64) {  // LD H, H
    EMIT_INSTRUCTION(pc_begin, "LD H, H");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::H, ArithmeticTarget::H, 4);
  } else if (opcode == 0x65) {  // LD H, L
    EMIT_INSTRUCTION(pc_begin, "LD H, L");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::H, ArithmeticTarget::L, 4);
  } else if (opcode == 0x66) {  // LD H, [HL]
    EMIT_INSTRUCTION(pc_begin, "LD H, [HL]");
    return std::make_unique<InstructionLoad>(ArithmeticTarget::H, LoadOperandType::REGISTER, ArithmeticTarget::HL, LoadOperandType::AS_ADDRESS, 8);
  } else if (opcode == 0x67) {  // LD H, A
    EMIT_INSTRUCTION(pc_begin, "LD H, A");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::H, ArithmeticTarget::A, 4);
  } else if (opcode == 0x68) {  // LD L, B
    EMIT_INSTRUCTION(pc_begin, "LD L, B");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::L, ArithmeticTarget::B, 4);
  } else if (opcode == 0x69) {  // LD L, C
    EMIT_INSTRUCTION(pc_begin, "LD L, C");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::L, ArithmeticTarget::C, 4);
  } else if (opcode == 0x6A) {  // LD L, D
    EMIT_INSTRUCTION(pc_begin, "LD L, D");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::L, ArithmeticTarget::D, 4);
  } else if (opcode == 0x6B) {  // LD L, E
    EMIT_INSTRUCTION(pc_begin, "LD L, E");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::L, ArithmeticTarget::E, 4);
  } else if (opcode == 0x6C) {  // LD L, H
    EMIT_INSTRUCTION(pc_begin, "LD L, H");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::L, ArithmeticTarget::H, 4);
  } else if (opcode == 0x6D) {  // LD L, L
    EMIT_INSTRUCTION(pc_begin, "LD L, L");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::L, ArithmeticTarget::L, 4);
  } else if (opcode == 0x6E) {  // LD L, [HL]
    EMIT_INSTRUCTION(pc_begin, "LD L, [HL]");
    return std::make_unique<InstructionLoad>(ArithmeticTarget::L, LoadOperandType::REGISTER, ArithmeticTarget::HL, LoadOperandType::AS_ADDRESS, 8);
  } else if (opcode == 0x6F) {  // LD L, A
    EMIT_INSTRUCTION(pc_begin, "LD L, A");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::L, ArithmeticTarget::A, 4);
  } else if (opcode == 0x70) {  // LD [HL], B
    EMIT_INSTRUCTION(pc_begin, "LD [HL], B");
    return std::make_unique<InstructionLoad>(ArithmeticTarget::HL, LoadOperandType::AS_ADDRESS, ArithmeticTarget::B, 8);
  } else if (opcode == 0x71) {  // LD [HL], C
    EMIT_INSTRUCTION(pc_begin, "LD [HL], C");
    return std::make_unique<InstructionLoad>(ArithmeticTarget::HL, LoadOperandType::AS_ADDRESS, ArithmeticTarget::C, 8);
  } else if (opcode == 0x72) {  // LD [HL], D
    EMIT_INSTRUCTION(pc_begin, "LD [HL], D");
    return std::make_unique<InstructionLoad>(ArithmeticTarget::HL, LoadOperandType::AS_ADDRESS, ArithmeticTarget::D, 8);
  } else if (opcode == 0x73) {  // LD [HL], E
    EMIT_INSTRUCTION(pc_begin, "LD [HL], E");
    return std::make_unique<InstructionLoad>(ArithmeticTarget::HL, LoadOperandType::AS_ADDRESS, ArithmeticTarget::E, 8);
  } else if (opcode == 0x74) {  // LD [HL], H
    EMIT_INSTRUCTION(pc_begin, "LD [HL], H");
    return std::make_unique<InstructionLoad>(ArithmeticTarget::HL, LoadOperandType::AS_ADDRESS, ArithmeticTarget::H, 8);
  } else if (opcode == 0x75) {  // LD [HL], L
    EMIT_INSTRUCTION(pc_begin, "LD [HL], L");
    return std::make_unique<InstructionLoad>(ArithmeticTarget::HL, LoadOperandType::AS_ADDRESS, ArithmeticTarget::L, 8);
  } else if (opcode == 0x76) {  // HALT
    EMIT_INSTRUCTION(pc_begin, "HALT");
    return std::make_unique<InstructionHalt>();
  } else if (opcode == 0x77) {  // LD [HL], A
    EMIT_INSTRUCTION(pc_begin, "LD [HL], A");
    return std::make_unique<InstructionLoad>(ArithmeticTarget::HL, LoadOperandType::AS_ADDRESS, ArithmeticTarget::A, 8);
  } else if (opcode == 0x78) {  // LD A, B
    EMIT_INSTRUCTION(pc_begin, "LD A, B");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::A, ArithmeticTarget::B, 4);
  } else if (opcode == 0x79) {  // LD A, C
    EMIT_INSTRUCTION(pc_begin, "LD A, C");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::A, ArithmeticTarget::C, 4);
  } else if (opcode == 0x7A) {  // LD A, D
    EMIT_INSTRUCTION(pc_begin, "LD A, D");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::A, ArithmeticTarget::D, 4);
  } else if (opcode == 0x7B) {  // LD A, E
    EMIT_INSTRUCTION(pc_begin, "LD A, E");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::A, ArithmeticTarget::E, 4);
  } else if (opcode == 0x7C) {  // LD A, H
    EMIT_INSTRUCTION(pc_begin, "LD A, H");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::A, ArithmeticTarget::H, 4);
  } else if (opcode == 0x7D) {  // LD A, L
    EMIT_INSTRUCTION(pc_begin, "LD A, L");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::A, ArithmeticTarget::L, 4);
  } else if (opcode == 0x7E) {  // LD A, [HL]
    EMIT_INSTRUCTION(pc_begin, "LD A, [HL]");
    return std::make_unique<InstructionLoad>(ArithmeticTarget::A, LoadOperandType::REGISTER, ArithmeticTarget::HL, LoadOperandType::AS_ADDRESS, 8);
  } else if (opcode == 0x7F) {  // LD A, A
    EMIT_INSTRUCTION(pc_begin, "LD A, A");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::A, ArithmeticTarget::A, 4);
  } else if (opcode == 0x80) {  // ADD A, B
    EMIT_INSTRUCTION(pc_begin, "ADD A, B");
    return std::make_unique<InstructionAdd>(ArithmeticTarget::A, ArithmeticTarget::B, false, 4);
  } else if (opcode == 0x81) {  // ADD A, C
    EMIT_INSTRUCTION(pc_begin, "ADD A, C");
    return std::make_unique<InstructionAdd>(ArithmeticTarget::A, ArithmeticTarget::C, false, 4);
  } else if (opcode == 0x82) {  // ADD A, D
    EMIT_INSTRUCTION(pc_begin, "ADD A, D");
    return std::make_unique<InstructionAdd>(ArithmeticTarget::A, ArithmeticTarget::D, false, 4);
  } else if (opcode == 0x83) {  // ADD A, E
    EMIT_INSTRUCTION(pc_begin, "ADD A, E");
    return std::make_unique<InstructionAdd>(ArithmeticTarget::A, ArithmeticTarget::E, false, 4);
  } else if (opcode == 0x84) {  // ADD A, H
    EMIT_INSTRUCTION(pc_begin, "ADD A, H");
    return std::make_unique<InstructionAdd>(ArithmeticTarget::A, ArithmeticTarget::H, false, 4);
  } else if (opcode == 0x85) {  // ADD A, L
    EMIT_INSTRUCTION(pc_begin, "ADD A, L");
    return std::make_unique<InstructionAdd>(ArithmeticTarget::A, ArithmeticTarget::L, false, 4);
  } else if (opcode == 0x86) {  // ADD A, [HL]
    EMIT_INSTRUCTION(pc_begin, "ADD A, [HL]");
    return std::make_unique<InstructionAdd>(ArithmeticTarget::A, ArithmeticTarget::HL, true, 8);
  } else if (opcode == 0x87) {  // ADD A, A
    EMIT_INSTRUCTION(pc_begin, "ADD A, A");
    return std::make_unique<InstructionAdd>(ArithmeticTarget::A, ArithmeticTarget::A, false, 4);
  } else if (opcode == 0x88) {  // ADC A, B
    EMIT_INSTRUCTION(pc_begin, "ADC A, B");
    return std::make_unique<InstructionAddCarry>(ArithmeticTarget::A, ArithmeticTarget::B, false, 4);
  } else if (opcode == 0x89) {  // ADC A, C
    EMIT_INSTRUCTION(pc_begin, "ADC A, C");
    return std::make_unique<InstructionAddCarry>(ArithmeticTarget::A, ArithmeticTarget::C, false, 4);
  } else if (opcode == 0x8A) {  // ADC A, D
    EMIT_INSTRUCTION(pc_begin, "ADC A, D");
    return std::make_unique<InstructionAddCarry>(ArithmeticTarget::A, ArithmeticTarget::D, false, 4);
  } else if (opcode == 0x8B) {  // ADC A, E
    EMIT_INSTRUCTION(pc_begin, "ADC A, E");
    return std::make_unique<InstructionAddCarry>(ArithmeticTarget::A, ArithmeticTarget::E, false, 4);
  } else if (opcode == 0x8C) {  // ADC A, H
    EMIT_INSTRUCTION(pc_begin, "ADC A, H");
    return std::make_unique<InstructionAddCarry>(ArithmeticTarget::A, ArithmeticTarget::H, false, 4);
  } else if (opcode == 0x8D) {  // ADC A, L
    EMIT_INSTRUCTION(pc_begin, "ADC A, L");
    return std::make_unique<InstructionAddCarry>(ArithmeticTarget::A, ArithmeticTarget::L, false, 4);
  } else if (opcode == 0x8E) {  // ADC A, [HL]
    EMIT_INSTRUCTION(pc_begin, "ADC A, [HL]");
    return std::make_unique<InstructionAddCarry>(ArithmeticTarget::A, ArithmeticTarget::HL, true, 8);
  } else if (opcode == 0x8F) {  // ADC A, A
    EMIT_INSTRUCTION(pc_begin, "ADC A, A");
    return std::make_unique<InstructionAddCarry>(ArithmeticTarget::A, ArithmeticTarget::A, false, 4);
  } else if (opcode == 0x90) {  // SUB A, B
    EMIT_INSTRUCTION(pc_begin, "SUB A, B");
    return std::make_unique<InstructionSub>(ArithmeticTarget::A, ArithmeticTarget::B, false, 4);
  } else if (opcode == 0x91) {  // SUB A, C
    EMIT_INSTRUCTION(pc_begin, "SUB A, C");
    return std::make_unique<InstructionSub>(ArithmeticTarget::A, ArithmeticTarget::C, false, 4);
  } else if (opcode == 0x92) {  // SUB A, D
    EMIT_INSTRUCTION(pc_begin, "SUB A, D");
    return std::make_unique<InstructionSub>(ArithmeticTarget::A, ArithmeticTarget::D, false, 4);
  } else if (opcode == 0x93) {  // SUB A, E
    EMIT_INSTRUCTION(pc_begin, "SUB A, E");
    return std::make_unique<InstructionSub>(ArithmeticTarget::A, ArithmeticTarget::E, false, 4);
  } else if (opcode == 0x94) {  // SUB A, H
    EMIT_INSTRUCTION(pc_begin, "SUB A, H");
    return std::make_unique<InstructionSub>(ArithmeticTarget::A, ArithmeticTarget::H, false, 4);
  } else if (opcode == 0x95) {  // SUB A, L
    EMIT_INSTRUCTION(pc_begin, "SUB A, L");
    return std::make_unique<InstructionSub>(ArithmeticTarget::A, ArithmeticTarget::L, false, 4);
  } else if (opcode == 0x96) {  // SUB A, [HL]
    EMIT_INSTRUCTION(pc_begin, "SUB A, [HL]");
    return std::make_unique<InstructionSub>(ArithmeticTarget::A, ArithmeticTarget::HL, true, 8);
  } else if (opcode == 0x97) {  // SUB A, A
    EMIT_INSTRUCTION(pc_begin, "SUB A, A");
    return std::make_unique<InstructionSub>(ArithmeticTarget::A, ArithmeticTarget::A, false, 4);
  } else if (opcode == 0x98) {  // SBC A, B
    EMIT_INSTRUCTION(pc_begin, "SBC A, B");
    return std::make_unique<InstructionSubCarry>(ArithmeticTarget::A, ArithmeticTarget::B, false, 4);
  } else if (opcode == 0x99) {  // SBC A, C
    EMIT_INSTRUCTION(pc_begin, "SBC A, C");
    return std::make_unique<InstructionSubCarry>(ArithmeticTarget::A, ArithmeticTarget::C, false, 4);
  } else if (opcode == 0x9A) {  // SBC A, D
    EMIT_INSTRUCTION(pc_begin, "SBC A, D");
    return std::make_unique<InstructionSubCarry>(ArithmeticTarget::A, ArithmeticTarget::D, false, 4);
  } else if (opcode == 0x9B) {  // SBC A, E
    EMIT_INSTRUCTION(pc_begin, "SBC A, E");
    return std::make_unique<InstructionSubCarry>(ArithmeticTarget::A, ArithmeticTarget::E, false, 4);
  } else if (opcode == 0x9C) {  // SBC A, H
    EMIT_INSTRUCTION(pc_begin, "SBC A, H");
    return std::make_unique<InstructionSubCarry>(ArithmeticTarget::A, ArithmeticTarget::H, false, 4);
  } else if (opcode == 0x9D) {  // SBC A, L
    EMIT_INSTRUCTION(pc_begin, "SBC A, L");
    return std::make_unique<InstructionSubCarry>(ArithmeticTarget::A, ArithmeticTarget::L, false, 4);
  } else if (opcode == 0x9E) {  // SBC A, [HL]
    EMIT_INSTRUCTION(pc_begin, "SBC A, [HL]");
    return std::make_unique<InstructionSubCarry>(ArithmeticTarget::A, ArithmeticTarget::HL, true, 8);
  } else if (opcode == 0x9F) {  // SBC A, A
    EMIT_INSTRUCTION(pc_begin, "SBC A, A");
    return std::make_unique<InstructionSubCarry>(ArithmeticTarget::A, ArithmeticTarget::A, false, 4);
  } else if (opcode == 0xA0) {  // AND A, B
    EMIT_INSTRUCTION(pc_begin, "AND A, B");
    return std::make_unique<InstructionAnd>(ArithmeticTarget::A, ArithmeticTarget::B, false, 4);
  } else if (opcode == 0xA1) {  // AND A, C
    EMIT_INSTRUCTION(pc_begin, "AND A, C");
    return std::make_unique<InstructionAnd>(ArithmeticTarget::A, ArithmeticTarget::C, false, 4);
  } else if (opcode == 0xA2) {  // AND A, D
    EMIT_INSTRUCTION(pc_begin, "AND A, D");
    return std::make_unique<InstructionAnd>(ArithmeticTarget::A, ArithmeticTarget::D, false, 4);
  } else if (opcode == 0xA3) {  // AND A, E
    EMIT_INSTRUCTION(pc_begin, "AND A, E");
    return std::make_unique<InstructionAnd>(ArithmeticTarget::A, ArithmeticTarget::E, false, 4);
  } else if (opcode == 0xA4) {  // AND A, H
    EMIT_INSTRUCTION(pc_begin, "AND A, H");
    return std::make_unique<InstructionAnd>(ArithmeticTarget::A, ArithmeticTarget::H, false, 4);
  } else if (opcode == 0xA5) {  // AND A, L
    EMIT_INSTRUCTION(pc_begin, "AND A, L");
    return std::make_unique<InstructionAnd>(ArithmeticTarget::A, ArithmeticTarget::L, false, 4);
  } else if (opcode == 0xA6) {  // AND A, [HL]
    EMIT_INSTRUCTION(pc_begin, "AND A, [HL]");
    return std::make_unique<InstructionAnd>(ArithmeticTarget::A, ArithmeticTarget::HL, true, 8);
  } else if (opcode == 0xA7) {  // AND A, A
    EMIT_INSTRUCTION(pc_begin, "AND A, A");
    return std::make_unique<InstructionAnd>(ArithmeticTarget::A, ArithmeticTarget::A, false, 4);
  } else if (opcode == 0xA8) {  // XOR A, B
    EMIT_INSTRUCTION(pc_begin, "XOR A, B");
    return std::make_unique<InstructionXOR>(ArithmeticTarget::A, ArithmeticTarget::B, false, 4);
  } else if (opcode == 0xA9) {  // XOR A, C
    EMIT_INSTRUCTION(pc_begin, "XOR A, C");
    return std::make_unique<InstructionXOR>(ArithmeticTarget::A, ArithmeticTarget::C, false, 4);
  } else if (opcode == 0xAA) {  // XOR A, D
    EMIT_INSTRUCTION(pc_begin, "XOR A, D");
    return std::make_unique<InstructionXOR>(ArithmeticTarget::A, ArithmeticTarget::D, false, 4);
  } else if (opcode == 0xAB) {  // XOR A, E
    EMIT_INSTRUCTION(pc_begin, "XOR A, E");
    return std::make_unique<InstructionXOR>(ArithmeticTarget::A, ArithmeticTarget::E, false, 4);
  } else if (opcode == 0xAC) {  // XOR A, H
    EMIT_INSTRUCTION(pc_begin, "XOR A, H");
    return std::make_unique<InstructionXOR>(ArithmeticTarget::A, ArithmeticTarget::H, false, 4);
  } else if (opcode == 0xAD) {  // XOR A, L
    EMIT_INSTRUCTION(pc_begin, "XOR A, L");
    return std::make_unique<InstructionXOR>(ArithmeticTarget::A, ArithmeticTarget::L, false, 4);
  } else if (opcode == 0xAE) {  // XOR A, [HL]
    EMIT_INSTRUCTION(pc_begin, "XOR A, [HL]");
    return std::make_unique<InstructionXOR>(ArithmeticTarget::A, ArithmeticTarget::HL, true, 8);
  } else if (opcode == 0xAF) {  // XOR A, A
    EMIT_INSTRUCTION(pc_begin, "XOR A, A");
    return std::make_unique<InstructionXOR>(ArithmeticTarget::A, ArithmeticTarget::A, false, 4);
  } else if (opcode == 0xB0) {  // OR A, B
    EMIT_INSTRUCTION(pc_begin, "OR A, B");
    return std::make_unique<InstructionOr>(ArithmeticTarget::A, ArithmeticTarget::B, false, 4);
  } else if (opcode == 0xB1) {  // OR A, C
    EMIT_INSTRUCTION(pc_begin, "OR A, C");
    return std::make_unique<InstructionOr>(ArithmeticTarget::A, ArithmeticTarget::C, false, 4);
  } else if (opcode == 0xB2) {  // OR A, D
    EMIT_INSTRUCTION(pc_begin, "OR A, D");
    return std::make_unique<InstructionOr>(ArithmeticTarget::A, ArithmeticTarget::D, false, 4);
  } else if (opcode == 0xB3) {  // OR A, E
    EMIT_INSTRUCTION(pc_begin, "OR A, E");
    return std::make_unique<InstructionOr>(ArithmeticTarget::A, ArithmeticTarget::E, false, 4);
  } else if (opcode == 0xB4) {  // OR A, H
    EMIT_INSTRUCTION(pc_begin, "OR A, H");
    return std::make_unique<InstructionOr>(ArithmeticTarget::A, ArithmeticTarget::H, false, 4);
  } else if (opcode == 0xB5) {  // OR A, L
    EMIT_INSTRUCTION(pc_begin, "OR A, L");
    return std::make_unique<InstructionOr>(ArithmeticTarget::A, ArithmeticTarget::L, false, 4);
  } else if (opcode == 0xB6) {  // OR A, [HL]
    EMIT_INSTRUCTION(pc_begin, "OR A, [HL]");
    return std::make_unique<InstructionOr>(ArithmeticTarget::A, ArithmeticTarget::HL, true, 8);
  } else if (opcode == 0xB7) {  // OR A, A
    EMIT_INSTRUCTION(pc_begin, "OR A, A");
    return std::make_unique<InstructionOr>(ArithmeticTarget::A, ArithmeticTarget::A, false, 4);
  } else if (opcode == 0xB8) {  // CP A, B
    EMIT_INSTRUCTION(pc_begin, "CP A, B");
    return std::make_unique<InstructionCompare>(ArithmeticTarget::A, ArithmeticTarget::B, false, 4);
  } else if (opcode == 0xB9) {  // CP A, C
    EMIT_INSTRUCTION(pc_begin, "CP A, C");
    return std::make_unique<InstructionCompare>(ArithmeticTarget::A, ArithmeticTarget::C, false, 4);
  } else if (opcode == 0xBA) {  // CP A, D
    EMIT_INSTRUCTION(pc_begin, "CP A, D");
    return std::make_unique<InstructionCompare>(ArithmeticTarget::A, ArithmeticTarget::D, false, 4);
  } else if (opcode == 0xBB) {  // CP A, E
    EMIT_INSTRUCTION(pc_begin, "CP A, E");
    return std::make_unique<InstructionCompare>(ArithmeticTarget::A, ArithmeticTarget::E, false, 4);
  } else if (opcode == 0xBC) {  // CP A, H
    EMIT_INSTRUCTION(pc_begin, "CP A, H");
    return std::make_unique<InstructionCompare>(ArithmeticTarget::A, ArithmeticTarget::H, false, 4);
  } else if (opcode == 0xBD) {  // CP A, L
    EMIT_INSTRUCTION(pc_begin, "CP A, L");
    return std::make_unique<InstructionCompare>(ArithmeticTarget::A, ArithmeticTarget::L, false, 4);
  } else if (opcode == 0xBE) {  // CP A, [HL]
    EMIT_INSTRUCTION(pc_begin, "CP A, [HL]");
    return std::make_unique<InstructionCompare>(ArithmeticTarget::A, ArithmeticTarget::HL, true, 8);
  } else if (opcode == 0xBF) {  // CP A, A
    EMIT_INSTRUCTION(pc_begin, "CP A, A");
    return std::make_unique<InstructionCompare>(ArithmeticTarget::A, ArithmeticTarget::A, false, 4);
  } else if (opcode == 0xC0) {  // RET NZ
    EMIT_INSTRUCTION(pc_begin, "RET NZ");
    return std::make_unique<InstructionReturnIfZero>(true);
  } else if (opcode == 0xC1) {  // POP BC
    EMIT_INSTRUCTION(pc_begin, "POP BC");
    return std::make_unique<InstructionPop>(ArithmeticTarget::BC);
  } else if (opcode == 0xC2) {  // JP NZ, a16
    u16 value = FetchWord(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "JP NZ, {}", ToHex(value));
    return std::make_unique<InstructionJumpIfZero>(value, true);
  } else if (opcode == 0xC3) {  // JP a16
    u16 value = FetchWord(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "JP {}", ToHex(value));
    return std::make_unique<InstructionJump>(value);
  } else if (opcode == 0xC4) {  // CALL NZ, a16
    u16 value = FetchWord(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "CALL NZ, {}", ToHex(value));
    return std::make_unique<InstructionCallIfZero>(value, true);
  } else if (opcode == 0xC5) {  // PUSH BC
    EMIT_INSTRUCTION(pc_begin, "PUSH BC");
    return std::make_unique<InstructionPush>(ArithmeticTarget::BC);
  } else if (opcode == 0xC6) {  // ADD A, n8
    u8 value = Fetch(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "ADD A, {}", ToHex(value));
    return std::make_unique<InstructionAddImmediate>(ArithmeticTarget::A, value, 8);
  } else if (opcode == 0xC7) {  // RST $00
    EMIT_INSTRUCTION(pc_begin, "RST $00");
    return std::make_unique<InstructionRestart>(0x0000);
  } else if (opcode == 0xC8) {  // RET Z
    EMIT_INSTRUCTION(pc_begin, "RET Z");
    return std::make_unique<InstructionReturnIfZero>(false);
  } else if (opcode == 0xC9) {  // RET
    EMIT_INSTRUCTION(pc_begin, "RET");
    return std::make_unique<InstructionReturn>(false);
  } else if (opcode == 0xCA) {  // JP Z, a16
    u16 value = FetchWord(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "JP Z, {}", ToHex(value));
    return std::make_unique<InstructionJumpIfZero>(value, false);
  } else if (opcode == 0xCB) {  // extended instructions
    return FetchPrefixed(alu, registers, bus);
  } else if (opcode == 0xCC) {  // CALL Z, a16
    u16 value = FetchWord(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "CALL Z, {}", ToHex(value));
    return std::make_unique<InstructionCallIfZero>(value, false);
  } else if (opcode == 0xCD) {  // CALL a16
    u16 value = FetchWord(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "CALL {}", ToHex(value));
    return std::make_unique<InstructionCall>(value);
  } else if (opcode == 0xCE) {  // ADD A, n8
    u8 value = Fetch(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "ADD A, {}", ToHex(value));
    return std::make_unique<InstructionAddCarryImmediate>(ArithmeticTarget::A, value, 8);
  } else if (opcode == 0xCF) {   // RST $08
    EMIT_INSTRUCTION(pc_begin, "RST $08");
    return std::make_unique<InstructionRestart>(0x0008);
  } else if (opcode == 0xD0) {  // RET NC
    EMIT_INSTRUCTION(pc_begin, "RET NC");
    return std::make_unique<InstructionReturnIfCarry>(true);
  } else if (opcode == 0xD1) {  // POP DE
    EMIT_INSTRUCTION(pc_begin, "POP DE");
    return std::make_unique<InstructionPop>(ArithmeticTarget::DE);
  } else if (opcode == 0xD2) {  // JP NC, a16
    u16 value = FetchWord(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "JP NC, {}", ToHex(value));
    return std::make_unique<InstructionJumpIfCarry>(value, true);
  } else if (opcode == 0xD3) {  // -
    //EMIT_INSTRUCTION(pc_begin, "INVALID INSTRUCTION");
    // todo hard lock cpu
  } else if (opcode == 0xD4) {  // CALL NC, a16
    u16 value = FetchWord(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "CALL NC, {}", ToHex(value));
    return std::make_unique<InstructionCallIfCarry>(value, true);
  } else if (opcode == 0xD5) {  // PUSH DE
    EMIT_INSTRUCTION(pc_begin, "PUSH DE");
    return std::make_unique<InstructionPush>(ArithmeticTarget::DE);
  } else if (opcode == 0xD6) {  // SUB A, n8
    u8 value = Fetch(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "SUB A, {}", ToHex(value));
    return std::make_unique<InstructionSubImmediate>(ArithmeticTarget::A, value, 8);
  } else if (opcode == 0xD7) {  // RST $10
    EMIT_INSTRUCTION(pc_begin, "RST $10");
    return std::make_unique<InstructionRestart>(0x0010);
  } else if (opcode == 0xD8) {  // RET C
    EMIT_INSTRUCTION(pc_begin, "RET C");
    return std::make_unique<InstructionReturnIfCarry>(false);
  } else if (opcode == 0xD9) {  // RETI
    EMIT_INSTRUCTION(pc_begin, "RETI");
    return std::make_unique<InstructionReturn>(true);
  } else if (opcode == 0xDA) {  // JP C, a16
    u16 value = FetchWord(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "JP C, {}", ToHex(value));
    return std::make_unique<InstructionJumpIfCarry>(value, false);
  } else if (opcode == 0xDB) {  // -
    //EMIT_INSTRUCTION(pc_begin, "INVALID INSTRUCTION");
    // todo hard lock cpu
  } else if (opcode == 0xDC) {  // CALL C, a16
    u16 value = FetchWord(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "CALL C, {}", ToHex(value));
    return std::make_unique<InstructionCallIfCarry>(value, false);
  } else if (opcode == 0xDD) {  // -
    //EMIT_INSTRUCTION(pc_begin, "INVALID INSTRUCTION");
    // todo hard lock cpu
  } else if (opcode == 0xDE) {  // SBC A, n8
    u8 value = Fetch(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "SBC A, {}", ToHex(value));
    return std::make_unique<InstructionSubCarryImmediate>(ArithmeticTarget::A, value, 8);
  } else if (opcode == 0xDF) {   // RST $18
    EMIT_INSTRUCTION(pc_begin, "RST $18");
    return std::make_unique<InstructionRestart>(0x0018);
  } else if (opcode == 0xE0) {  // LDH [a8], A
    u8 value = Fetch(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "LDH [{}], A", ToHex(value));
    return std::make_unique<InstructionLDH1>(value, ArithmeticTarget::A);
  } else if (opcode == 0xE1) {  // POP HL
    EMIT_INSTRUCTION(pc_begin, "POP HL");
    return std::make_unique<InstructionPop>(ArithmeticTarget::HL);
  } else if (opcode == 0xE2) {  // LD [C], A
    EMIT_INSTRUCTION(pc_begin, "LD [$FF00 + C], A");
    u8 value = registers.Get(ArithmeticTarget::C);
    return std::make_unique<InstructionLDH3>(value);
    //return std::make_unique<InstructionLoad>(ArithmeticTarget::C, LoadOperandType::AS_ADDRESS, ArithmeticTarget::A, LoadOperandType::REGISTER, 8);
  } else if (opcode == 0xE3) {  // -
    //EMIT_INSTRUCTION(pc_begin, "INVALID INSTRUCTION");
    // todo hard lock cpu
  } else if (opcode == 0xE4) {  // -
    //EMIT_INSTRUCTION(pc_begin, "INVALID INSTRUCTION");
    // todo hard lock cpu
  } else if (opcode == 0xE5) {  // PUSH HL
    EMIT_INSTRUCTION(pc_begin, "PUSH HL");
    return std::make_unique<InstructionPush>(ArithmeticTarget::HL);
  } else if (opcode == 0xE6) {  // AND A, n8
    u8 value = Fetch(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "AND A, {}", ToHex(value));
    return std::make_unique<InstructionAndImmediate>(ArithmeticTarget::A, value, 8);
  } else if (opcode == 0xE7) {  // RST $20
    EMIT_INSTRUCTION(pc_begin, "RST $20");
    return std::make_unique<InstructionRestart>(0x0020);
  } else if (opcode == 0xE8) {  // ADD SP, e8
    s8 value = AsSigned(Fetch(registers, bus));
    EMIT_INSTRUCTION(pc_begin, "ADD SP, {}", ToHex(value));
    return std::make_unique<InstructionAddSPImmediate>(value);
  } else if (opcode == 0xE9) {  // JP HL
    // todo move get to inside of the instruction!
    u16 value = registers.Get(ArithmeticTarget::HL);
    EMIT_INSTRUCTION(pc_begin, "JP HL");
    return std::make_unique<InstructionJump>(value);
  } else if (opcode == 0xEA) {  // LD [a16], A
    u16 value = FetchWord(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "LD [{}], A", ToHex(value));
    return std::make_unique<InstructionLoadToAddress>(value, ArithmeticTarget::A, 16);
  } else if (opcode == 0xEB) {  // -
    //EMIT_INSTRUCTION(pc_begin, "INVALID INSTRUCTION");
    // todo hard lock cpu
  } else if (opcode == 0xEC) {  // -
    //EMIT_INSTRUCTION(pc_begin, "INVALID INSTRUCTION");
    // todo hard lock cpu
  } else if (opcode == 0xED) {  // -
    //EMIT_INSTRUCTION(pc_begin, "INVALID INSTRUCTION");
    // todo hard lock cpu
  } else if (opcode == 0xEE) {  // XOR A, n8
    u8 value = Fetch(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "XOR A, {}", ToHex(value));
    return std::make_unique<InstructionXORImmediate>(ArithmeticTarget::A, value, 8);
  } else if (opcode == 0xEF) {   // RST $28
    EMIT_INSTRUCTION(pc_begin, "RST $28");
    return std::make_unique<InstructionRestart>(0x0028);
  } else if (opcode == 0xF0) {  // LDH A, [a8]
    u8 value = Fetch(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "LDH A, [{}]", ToHex(value));
    return std::make_unique<InstructionLDH2>(ArithmeticTarget::A, value);
  } else if (opcode == 0xF1) {  // POP AF
    EMIT_INSTRUCTION(pc_begin, "POP AF");
    return std::make_unique<InstructionPopAF>();
  } else if (opcode == 0xF2) {  // LD A, [C]
    EMIT_INSTRUCTION(pc_begin, "LD A, [$FF00 + C]");
    u16 value = registers.Get(ArithmeticTarget::C);
    return std::make_unique<InstructionLDH4>(value);
    //return std::make_unique<InstructionLoad>(ArithmeticTarget::A, LoadOperandType::REGISTER, ArithmeticTarget::C, LoadOperandType::AS_ADDRESS, 8);
  } else if (opcode == 0xF3) {  // DI
    EMIT_INSTRUCTION(pc_begin, "DI");
    return std::make_unique<InstructionDisableInterrupt>();
  } else if (opcode == 0xF4) {  // -
    //EMIT_INSTRUCTION(pc_begin, "INVALID INSTRUCTION");
    // todo hard lock cpu
  } else if (opcode == 0xF5) {  // PUSH AF
    EMIT_INSTRUCTION(pc_begin, "PUSH AF");
    return std::make_unique<InstructionPushAF>();
  } else if (opcode == 0xF6) {  // OR A, n8
    u8 value = Fetch(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "OR A, {}", ToHex(value));
    return std::make_unique<InstructionOrImmediate>(ArithmeticTarget::A, value, 8);
  } else if (opcode == 0xF7) {  // RST $30
    EMIT_INSTRUCTION(pc_begin, "RST $30");
    return std::make_unique<InstructionRestart>(0x0030);
  } else if (opcode == 0xF8) {  // LD HL, SP + e8
    s8 value = AsSigned(Fetch(registers, bus));
    EMIT_INSTRUCTION(pc_begin, "LD HL, SP + {}", ToHex(value));
    return std::make_unique<InstructionLoadHLSPImmediate>(value);
  } else if (opcode == 0xF9) {  // LD SP, HL
    EMIT_INSTRUCTION(pc_begin, "LD SP, HL");
    return std::make_unique<InstructionLoadRegisterToRegister>(ArithmeticTarget::SP, ArithmeticTarget::HL, 8);
  } else if (opcode == 0xFA) {  // LD A, [a16]
    u16 value = FetchWord(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "LD A, [{}]", ToHex(value));
    return std::make_unique<InstructionLoadImmediateAddress>(ArithmeticTarget::A, value, 16);
  } else if (opcode == 0xFB) {  // EI
    EMIT_INSTRUCTION(pc_begin, "EI");
    return std::make_unique<InstructionEnableInterrupt>();
  } else if (opcode == 0xFC) {  // -
    //EMIT_INSTRUCTION(pc_begin, "INVALID INSTRUCTION");
    // todo hard lock cpu
  } else if (opcode == 0xFD) {  // -
    //EMIT_INSTRUCTION(pc_begin, "INVALID INSTRUCTION");
    // todo hard lock cpu
  } else if (opcode == 0xFE) {  // CP A, n8
    u8 value = Fetch(registers, bus);
    EMIT_INSTRUCTION(pc_begin, "CP A, {}", ToHex(value));
    return std::make_unique<InstructionCompareImmediate>(ArithmeticTarget::A, value, 8);
  } else if (opcode == 0xFF) {   // RST $38
    EMIT_INSTRUCTION(pc_begin, "RST $38");
    return std::make_unique<InstructionRestart>(0x0038);
  }
  //std::cout << "unknown instruction: " << ToHex(opcode) << std::endl;
  return nullptr;
}
