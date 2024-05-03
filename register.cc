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
  std::cout << "a: " << ToBinary(a) << " (" << ToHex(a) << ")" << std::endl;
  std::cout << "b: " << ToBinary(bc.f.hi) << " (" << ToHex(bc.f.hi) << ")" << std::endl;
  std::cout << "c: " << ToBinary(bc.f.lo) << " (" << ToHex(bc.f.lo) << ")" << std::endl;
  std::cout << "d: " << ToBinary(de.f.hi) << " (" << ToHex(de.f.hi) << ")" << std::endl;
  std::cout << "e: " << ToBinary(de.f.lo) << " (" << ToHex(de.f.lo) << ")" << std::endl;
  std::cout << "h: " << ToBinary(hl.f.hi) << " (" << ToHex(hl.f.hi) << ")" << std::endl;
  std::cout << "l: " << ToBinary(hl.f.lo) << " (" << ToHex(hl.f.lo) << ")" << std::endl;
  std::cout << "sp: " << ToBinary(sp) << " (" << ToHex(sp) << ")" << std::endl;
  std::cout << "pc: " << ToBinary(pc) << " (" << ToHex(pc) << ")" << std::endl;
  std::cout << std::endl;
  std::cout << "flags: " << std::endl;
  std::cout << "f: " << ToBinary(flags.v) << " (" << ToHex(flags.v) << ")" << std::endl;
  std::cout << "zero: " << BoolToStr(flags.f.zero) << std::endl;
  std::cout << "subtract: " << BoolToStr(flags.f.subtract) << std::endl;
  std::cout << "half carry: " << BoolToStr(flags.f.half_carry) << std::endl;
  std::cout << "carry: " << BoolToStr(flags.f.carry) << std::endl;
}