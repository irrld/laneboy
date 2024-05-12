#include "debug.h"
#include "instructions.h"
#include <unordered_set>

#ifdef ENABLE_DEBUGGER

namespace Debugger {
// todo sync
static std::unordered_map<u16, std::string> disassemble_;
static std::array<u16, 0x10000> memory_cache_; // todo utilize this to prevent disassembling instructions over and over again
static std::unordered_set<u16> breakpoints_;
static std::unordered_map<u16, u8> lengths_;
static u16 current_ = 0;
static bool next_ = true;
static bool lock_ = true;
static u16 previous_write_address_ = 0;
static u8 previous_write_value_ = 0;
static std::mutex mutex_;
static std::mutex disassemble_mutex_;
static bool instructions_changed_ = false;

void Reset() {
  std::scoped_lock lock2{disassemble_mutex_, mutex_};
  lock_ = false;
  memory_cache_.fill(0);
  disassemble_.clear();
  lengths_.clear();
  current_ = 0;
  next_ = true;
  previous_write_address_ = 0;
  previous_write_value_ = 0;
  instructions_changed_ = true;
}

void DisassembleFromMemory(MemoryBus& bus) {
  {
    std::scoped_lock lock{disassemble_mutex_, mutex_};
    disassemble_.clear();
    lengths_.clear();
  }
  bool panic = bus.panic_on_invalid_access();
  bus.panic_on_invalid_access(false);
  EventBus fake_event_bus{};
  Registers registers{};
  ALU alu{registers};
  registers.pc = 0;
  while (registers.pc < 0xFFFF) {
    if (!bus.CheckAccess(registers.pc + 1, kMemoryAccessRead)) {
      if (((u32)registers.pc + 1) > 0xFFFF) {
        break;
      }
      registers.pc = registers.pc + 1;
      continue;
    }
    u16 start = registers.pc;
    Fetch(alu, registers, bus);
    u16 end = registers.pc;
    if (end < start) {
      break; // overflow
    }
  }
  bus.panic_on_invalid_access(panic);
}
void Init(MemoryBus& bus) {
  next_ = true;
  current_ = 0;
  DisassembleFromMemory(bus);
}

void OnEmitInstruction(MemoryBus& bus, u16 pc, u16 n, std::string name) {
  std::vector<std::string> bytes;
  u16 i = 0;
  while (i < n) {
    bytes.push_back(ToHex(bus.Read(pc + i), false));
    i++;
  }
  while (bytes.size() < 4) {
    bytes.emplace_back("  ");
  }
  std::string out;
  for (const auto& item : bytes) {
    out += item;
    out += " ";
  }
  out.pop_back();
  out += " : ";
  out += name;
  std::scoped_lock lock{disassemble_mutex_, mutex_}; // we lock before doing any changes
  if (disassemble_.contains(pc) && disassemble_[pc] == out) {
    current_ = pc;
    return;
  }
  instructions_changed_ = true;
  disassemble_[pc] = out;
  lengths_[pc] = n;
  for (int j = 1; j < n; ++j) {
    lengths_[pc + j] = 0;
  }
  current_ = pc;
}

void OnPreExecInstruction() {
  if (next_ || HasBreakpoint(current_)) {
    PauseHere();
  }
}

void OnPostExecInstruction() {

}

void OnMemWrite(MemoryBus& bus, u16 pos, u8 oldvalue, u8 value, u8 newvalue) {
  if (oldvalue == newvalue) {
    return;
  }
  previous_write_address_ = pos;
  previous_write_value_ = oldvalue;
  u16 current_old = current_;
  bool panic = bus.panic_on_invalid_access();
  bus.panic_on_invalid_access(false);
  EventBus fake_event_bus{};
  Registers registers{};
  registers.pc = pos;
  ALU alu{registers};
  if (!bus.CheckAccess(registers.pc, kMemoryAccessRead)) {
    bus.panic_on_invalid_access(panic);
    return;
  }
  Fetch(alu, registers, bus);
  bus.panic_on_invalid_access(panic);
  current_ = current_old;
}

void OnMemRead(MemoryBus& bus, u16 pos, u8 value) {

}

void OnCall(u16 pc, u16 sp, u16 value) {
  //std::cout << "call: " << ToHex(value) << ", return address: " << ToHex(current_) << ", sp: " << ToHex(sp) << std::endl;
}

void OnReturn(u16 pc, u16 sp, u16 value, bool from_interrupt) {
  //std::cout << "return: " << ToHex(value) << ", current address: " << ToHex(current_) << ", sp: " << ToHex(sp) << ", int: " << BoolToStr(from_interrupt) << std::endl;
}

void OnJump(u16 pc, u16 sp, u16 value) {
  //std::cout << "jump: " << ToHex(value) << " current address: " << ToHex(pc) << std::endl;
}

void OnJumpRelative(u16 pc, u16 sp, u16 value) {
  //std::cout << "jump relative: " << ToHex(value) << " current address: " << ToHex(pc) << std::endl;
}

void OnBankChange(MemoryBus& bus) {
  u16 current_temp = current_;
  DisassembleFromMemory(bus);
  current_ = current_temp;
}

void OnRomUnmap(MemoryBus& bus) {
  u16 current_temp = current_;
  DisassembleFromMemory(bus);
  current_ = current_temp;
}

const std::string& GetInstructionAt(u16 address) {
  std::scoped_lock lock{disassemble_mutex_};
  static std::string blank = "";
  if (disassemble_.contains(address)) {
    return disassemble_[address];
  }
  return blank;
}

u16 GetCurrentInstruction() {
  return current_;
}

u8 GetInstructionLengthAt(u16 address) {
  std::scoped_lock lock{disassemble_mutex_};
  u8 len = 0;
  if (lengths_.contains(address)) {
    len = lengths_[address];
  }
  return len;
}

bool HasBreakpoint(u16 address) {
  return breakpoints_.find(address) != breakpoints_.end();
}

void SetBreakpoint(u16 address, bool enabled) {
  if (enabled) {
    breakpoints_.insert(address);
  } else {
    breakpoints_.erase(address);
  }
}

bool IsFrozen() {
  return lock_;
}

void Step() {
  std::scoped_lock lock{mutex_};
  lock_ = false;
  next_ = true;
}

void Pause() {
  std::scoped_lock lock{mutex_};
  next_ = true;
}

void PauseHere() {
  {
    std::scoped_lock lock{mutex_};
    lock_ = true;
    next_ = false;
  }
  while (lock_) { }
}

void Continue() {
  std::scoped_lock lock{mutex_};
  lock_ = false;
}


u8 GetPreviousWrittenValue() {
    return previous_write_value_;
}

u16 GetPreviousWrittenAddress() {
  return previous_write_address_;
}

bool CheckInstructionsChangedAndClear() {
  bool value = instructions_changed_;
  instructions_changed_ = false;
  return value;
}
}


#endif