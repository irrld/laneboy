#include "emulator.h"
#include "instructions.h"
#include "test_opcodes.cc"

int main() {
  Emulator emulator;
  emulator.Start();
  //test_opcodes();
}

