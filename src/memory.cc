#include "memory.h"
#include "debug.h"

MemoryBus::MemoryBus() {
  lock_map_.fill(false);
}

void MemoryBus::Reset() {
  lock_map_ = {};
  map_ = {};
}

u8 MemoryBus::Read(u16 address) {
  MemoryDevice* device = SelectDevice(address);
  if (!device) {
    if (!panic_on_invalid_access_) {
      return 0xFF;
    }
    // todo implement a panic event that can be passed safely
    // to the cpu and handled there instead of crashing the emulator
    std::cerr << "cannot read from memory address " << ToHex(address) << "!" << std::endl;
    DEBUGGER_PAUSE_HERE();
    std::exit(EXIT_FAILURE);
  }
  if (!device->CheckAccess(address, kMemoryAccessRead)) {
    return 0xFF;
  }
  return device->Read(address);
}

u16 MemoryBus::ReadWord(u16 address) {
  return Read(address) | ((u16)Read(address + 1) << 8);
}

void MemoryBus::Write(u16 address, u8 value) {
  MemoryDevice* device = SelectDevice(address);
  if (!device) {
    if (!panic_on_invalid_access_) {
      return;
    }
    // todo implement a panic event that can be passed safely
    // to the cpu and handled there instead of crashing the emulator
    std::cerr << "cannot write value " << ToHex(value) << " to memory address " << ToHex(address) << "!" << std::endl;
    DEBUGGER_PAUSE_HERE();
    std::exit(EXIT_FAILURE);
  }
  if (!device->CheckAccess(address, kMemoryAccessWrite)) {
    device->OnFailedWrite(address, value);
    return;
  }
#ifdef ENABLE_DEBUGGER
  u8 previous_value = device->Read(address);
  device->Write(address, value);
  u8 new_value = device->Read(address);
  EMIT_MEM_WRITE(address, previous_value, value, new_value);
#else
  device->Write(address, value);
#endif
}


void MemoryBus::WriteWord(u16 address, u16 value) {
  // little endian
  Write(address, (u8)(value & 0x00FF));
  Write(address + 1, (u8)((value & 0xFF00) >> 8));
}

bool MemoryBus::CheckAccess(u16 address, MemoryAccess access) {
  MemoryDevice* device = SelectDevice(address);
  if (!device || !device->CheckAccess(address, access)) {
    return false;
  }
  return true;
}

void MemoryBus::AddDevice(u16 address, MemoryDevice* device, bool lock) {
  if (lock_map_[address]) {
    abort(); // cannot push to a locked memory location
  }
  map_[address].push_back(device);
  if (lock) {
    lock_map_[address] = true;
  }
}

void MemoryBus::AddDevice(u16 start_address, u16 end_address, MemoryDevice* device, bool lock) {
  for (u32 address = start_address; address <= end_address; address++) {
    AddDevice(address, device, lock);
  }
}

void MemoryBus::PopFrontDevice(u16 address) {
  if (lock_map_[address]) {
    abort(); // cannot pop a locked memory location
  }
  std::deque<MemoryDevice*>& q = map_[address];
  if (q.empty()) {
    return; // maybe error here?
  }
  q.pop_front();
}

void MemoryBus::PopFrontDevice(u16 start_address, u16 end_address) {
  for (u16 address = start_address; address <= end_address; address++) {
    PopFrontDevice(address);
  }
}

MemoryDevice* MemoryBus::SelectDevice(u16 address) {
  // select the first one in the deque
  const std::deque<MemoryDevice*>& q = map_[address];
  if (q.empty()) {
    return nullptr;
  }
  return q.front();
}