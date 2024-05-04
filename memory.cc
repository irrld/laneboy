#include "memory.h"
#include "debug.h"

MemoryBus::MemoryBus() {
  lock_map_.fill(false);
}

u8 MemoryBus::Read(u16 address) {
  MemoryDevice* device = SelectDevice(address);
  if (!device || !device->CheckAccess(address, AccessType::Read)) {
    if (!panic_on_invalid_access_) {
      return 0x00;
    }
    // todo implement a panic event that can be passed safely
    // to the cpu and handled there instead of crashing the emulator
    abort();
  }
  return device->Read(address);
}

u16 MemoryBus::ReadWord(u16 address) {
  return (((u16)Read(address + 1) << 8) | Read(address));
}

void MemoryBus::Write(u16 address, u8 value) {
  MemoryDevice* device = SelectDevice(address);
  if (!device || !device->CheckAccess(address, AccessType::Write)) {
    if (!panic_on_invalid_access_) {
      return;
    }
    // todo implement a panic event that can be passed safely
    // to the cpu and handled there instead of crashing the emulator
    abort();
  }
  device->Write(address, value);
}

void MemoryBus::WriteWord(u16 address, u16 value) {
  // little endian
  Write(address, value & 0xFF);
  Write(address + 1, value >> 8);
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
  for (u16 address = start_address; address < end_address; address++) {
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
  for (u16 address = start_address; address < end_address; address++) {
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