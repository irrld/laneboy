#pragma once

#include "cpu.h"

#ifdef DEBUG

namespace Debugger {
  void Init(CPU& cpu);

  void OnEmitInstruction(u16 pc, std::string name);
}
#endif


#ifdef DEBUG
#define INIT_DEBUGGER(cpu) Debugger::Init(cpu)
#define EMIT_INSTRUCTION(pc, name, args...) Debugger::OnEmitInstruction(pc, fmt::format(name, ##args))
#define EMIT_PRE_EXEC_INSTRUCTION()
#define EMIT_POST_EXEC_INSTRUCTION()
#define EMIT_MEM_WRITE(pos, oldval, newval)
#define EMIT_MEM_READ(pos, value)
#define EMIT_ROM_READ(pos, value)
#define EMIT_ROM_UNMAP()
#else
#define INIT_DEBUGGER(cpu)
#define EMIT_INSTRUCTION(pc, name, args)
#define EMIT_PRE_EXEC_INSTRUCTION()
#define EMIT_POST_EXEC_INSTRUCTION()
#define EMIT_MEM_WRITE(pos, oldval, newval)
#define EMIT_MEM_READ(pos, value)
#define EMIT_ROM_READ(pos, value)
#define EMIT_ROM_UNMAP()
#endif
