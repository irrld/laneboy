#include "register.h"

bool is_16bit_register(ArithmeticTarget target) {
  return target == ArithmeticTarget::BC || target == ArithmeticTarget::DE ||
    target == ArithmeticTarget::HL || target == ArithmeticTarget::SP;
}


void Registers::Set(ArithmeticTarget target, u16 v) {
  switch (target) {
    case ArithmeticTarget::A:
      a = v & 0xFF;
      break;
    case ArithmeticTarget::B:
      bc.f.hi = v & 0xFF;
      break;
    case ArithmeticTarget::C:
      bc.f.lo = v & 0xFF;
      break;
    case ArithmeticTarget::D:
      de.f.hi = v & 0xFF;
      break;
    case ArithmeticTarget::E:
      de.f.lo = v & 0xFF;
      break;
    case ArithmeticTarget::H:
      hl.f.hi = v & 0xFF;
      break;
    case ArithmeticTarget::L:
      hl.f.lo = v & 0xFF;
      break;
    case ArithmeticTarget::BC:
      bc.v = v;
      break;
    case ArithmeticTarget::DE:
      de.v = v;
      break;
    case ArithmeticTarget::HL:
      hl.v = v;
      break;
    case ArithmeticTarget::SP:
      sp = v;
      break;
  }
}

u16 Registers::Get(ArithmeticTarget target) const {
  switch (target) {
    case ArithmeticTarget::A:
      return a;
    case ArithmeticTarget::B:
      return bc.f.hi;
    case ArithmeticTarget::C:
      return bc.f.lo;
    case ArithmeticTarget::D:
      return de.f.hi;
    case ArithmeticTarget::E:
      return de.f.lo;
    case ArithmeticTarget::H:
      return hl.f.hi;
    case ArithmeticTarget::L:
      return hl.f.lo;
    case ArithmeticTarget::BC:
      return bc.v;
    case ArithmeticTarget::DE:
      return de.v;
    case ArithmeticTarget::HL:
      return hl.v;
    case ArithmeticTarget::SP:
      return sp;
  }
}

void Registers::Print() {
  std::cout << "a: 0b" << number_to_binary(a) << " (" << std::to_string(a) << ")" << std::endl;
  std::cout << "b: 0b" << number_to_binary(bc.f.hi) << " (" << std::to_string(bc.f.hi) << ")" << std::endl;
  std::cout << "c: 0b" << number_to_binary(bc.f.lo) << " (" << std::to_string(bc.f.lo) << ")" << std::endl;
  std::cout << "d: 0b" << number_to_binary(de.f.hi) << " (" << std::to_string(de.f.hi) << ")" << std::endl;
  std::cout << "e: 0b" << number_to_binary(de.f.lo) << " (" << std::to_string(de.f.lo) << ")" << std::endl;
  std::cout << "h: 0b" << number_to_binary(hl.f.hi) << " (" << std::to_string(hl.f.hi) << ")" << std::endl;
  std::cout << "l: 0b" << number_to_binary(hl.f.lo) << " (" << std::to_string(hl.f.lo) << ")" << std::endl;
  std::cout << "sp: 0b" << number_to_binary(sp) << " (" << std::to_string(sp) << ")" << std::endl;
  std::cout << "pc: 0b" << number_to_binary(pc) << " (" << std::to_string(pc) << ")" << std::endl;
  std::cout << std::endl;
  std::cout << "flags: " << std::endl;
  std::cout << "f: 0b" << number_to_binary(flags.v) << " (" << std::to_string(flags.v) << ")" << std::endl;
  std::cout << "zero: " << bool_to_str(flags.f.zero) << std::endl;
  std::cout << "subtract: " << bool_to_str(flags.f.subtract) << std::endl;
  std::cout << "half carry: " << bool_to_str(flags.f.half_carry) << std::endl;
  std::cout << "carry: " << bool_to_str(flags.f.carry) << std::endl;
}