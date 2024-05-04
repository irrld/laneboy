#pragma once

#include "cpu.h"

#ifdef DEBUG

namespace Debugger {
  void Init(CPU& cpu);

  void OnEmitInstruction(u16 pc, std::string name);

  void OnPreExecInstruction();
  void OnPostExecInstruction();
  void OnMemWrite(u16 pos, u8 oldval, u8 newval);
  void OnMemRead(u16 pos, u8 val);
  void OnRomRead(u16 pos, u8 val);
  void OnRomUnmap();
}
#endif


#ifdef DEBUG
#define INIT_DEBUGGER(cpu) Debugger::Init(cpu)
#define EMIT_INSTRUCTION(pc, name, args...) Debugger::OnEmitInstruction(pc, fmt::format(name, ##args))
#define EMIT_PRE_EXEC_INSTRUCTION() Debugger::OnPreExecInstruction()
#define EMIT_POST_EXEC_INSTRUCTION() Debugger::OnPostExecInstruction()
#define EMIT_MEM_WRITE(pos, oldval, newval) Debugger::OnMemWrite(pos, oldval, newval)
#define EMIT_MEM_READ(pos, value) Debugger::OnMemRead(pos, value)
#define EMIT_ROM_READ(pos, value) Debugger::OnRomRead(pos, value)
#define EMIT_ROM_UNMAP() Debugger::OnRomUnmap()
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
