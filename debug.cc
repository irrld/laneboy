#include "debug.h"
#include "instructions.h"

#ifdef DEBUG

namespace Debugger {
void Init(CPU& cpu) {
  /*CPU new_cpu;
  new_cpu.bus_.SetNoUninitializedRAM(false);
  memcpy(&new_cpu, &cpu, sizeof(CPU));

  while (new_cpu.registers_.pc < 0xFFFF) {
    Fetch(new_cpu, new_cpu.registers_, new_cpu.bus_);
  }*/
  //exit(0);
}

void OnEmitInstruction(u16 pc, std::string name) {
  std::cout << ToHex(pc) << "\t: " + name << std::endl;
}
}

#endif