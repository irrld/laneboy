#pragma once

#include "memory.h"

#ifdef DEBUG
#define ENABLE_DEBUGGER
#endif

#ifdef ENABLE_DEBUGGER

namespace Debugger {

  void Reset();
  void Init(MemoryBus& bus);

  void OnEmitInstruction(MemoryBus& bus, u16 pc, u16 n, std::string name);

  void OnPreExecInstruction();
  void OnPostExecInstruction();
  void OnMemWrite(MemoryBus& bus, u16 pos, u8 oldvalue, u8 value, u8 newvalue);
  void OnMemRead(MemoryBus& bus, u16 pos, u8 value);
  void OnRomUnmap(MemoryBus& bus);
  void OnCall(u16 pc, u16 sp, u16 value);
  void OnReturn(u16 pc, u16 sp, u16 value, bool from_interrupt);
  void OnJump(u16 pc, u16 sp, u16 value);
  void OnJumpRelative(u16 pc, u16 sp, u16 value);
  void OnBankChange(MemoryBus& bus);

  const std::string& GetInstructionAt(u16 address);

  u16 GetCurrentInstruction();
  u8 GetInstructionLengthAt(u16 address);

  bool HasBreakpoint(u16 address);

  void SetBreakpoint(u16 address, bool enabled);

  bool IsFrozen();
  void Step();
  void Pause();
  void PauseHere();
  void Continue();

  u8 GetPreviousWrittenValue();
  u16 GetPreviousWrittenAddress();

  bool CheckInstructionsChangedAndClear();
}
#endif


#ifdef ENABLE_DEBUGGER
#define INIT_DEBUGGER(memory) Debugger::Init(memory)
#define RESET_DEBUGGER(memory) Debugger::Reset()
#define EMIT_INSTRUCTION(begin_pc, name, args...) Debugger::OnEmitInstruction(bus, begin_pc, registers.pc - begin_pc, fmt::format(name, ##args))
#define EMIT_PRE_EXEC_INSTRUCTION() Debugger::OnPreExecInstruction()
#define EMIT_POST_EXEC_INSTRUCTION() Debugger::OnPostExecInstruction()
#define EMIT_MEM_WRITE(pos, oldvalue, value, newvalue) Debugger::OnMemWrite(*this, pos, oldvalue, value, newvalue)
#define EMIT_MEM_READ(pos, value) Debugger::OnMemRead(*this, pos, value)
#define EMIT_ROM_UNMAP(bus) Debugger::OnRomUnmap(bus)
#define EMIT_CALL(pc, sp, value) Debugger::OnCall(pc, sp, value)
#define EMIT_RET(pc, sp, value, from_interrupt) Debugger::OnReturn(pc, sp, value, from_interrupt)
#define EMIT_JUMP_RELATIVE(pc, sp, value) Debugger::OnJumpRelative(pc, sp, value)
#define EMIT_JUMP(pc, sp, value) Debugger::OnJump(pc, sp, value)
#define DEBUGGER_PAUSE() Debugger::Pause()
#define DEBUGGER_PAUSE_HERE() Debugger::PauseHere()
#define EMIT_BANK_CHANGE(bus) Debugger::OnBankChange(bus)
#else
#define INIT_DEBUGGER(memory)
#define RESET_DEBUGGER()
#define EMIT_INSTRUCTION(begin_pc, name, args...)
#define EMIT_PRE_EXEC_INSTRUCTION()
#define EMIT_POST_EXEC_INSTRUCTION()
#define EMIT_MEM_WRITE(pos, oldvalue, value, newvalue)
#define EMIT_MEM_READ(pos, value)
#define EMIT_ROM_UNMAP(bus)
#define EMIT_CALL(pc, sp, value)
#define EMIT_RET(pc, sp, value, from_interrupt)
#define EMIT_JUMP_RELATIVE(pc, sp, value)
#define EMIT_JUMP(pc, sp, value)
#define DEBUGGER_PAUSE()
#define DEBUGGER_PAUSE_HERE()
#define EMIT_BANK_CHANGE(bus)
#endif
