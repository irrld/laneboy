#include "cpu.h"

int main() {

  CPU cpu;
  cpu.registers_.Set(ArithmeticTarget::A, 255);
  cpu.registers_.Set(ArithmeticTarget::B, 10);

  //InstructionAdd(ArithmeticTarget::A, ArithmeticTarget::B, 4).Execute(cpu);
  cpu.registers_.Print();

}
