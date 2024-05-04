#pragma once

#include "util.h"
#include <stack>
#include <utility>

// Virtual Memory System


enum class AccessType { Read, Write };

class MemoryDevice {
 public:
  virtual u8 Read(u16 address) = 0;

  virtual void Write(u16 address, u8 value) = 0;

  virtual bool CheckAccess(u16 address, AccessType type) = 0;

};

class SinglePointerMemoryDevice : public MemoryDevice {
 public:
  SinglePointerMemoryDevice(u8* ptr, bool read, bool write) : ptr_(ptr), read_(read), write_(write) {}

  u8 Read(u16 address) override {
    return *ptr_;
  }

  void Write(u16 address, u8 value) override {
    *ptr_ = value;
  }

  bool CheckAccess(u16 address, AccessType type) override {
    if (type == AccessType::Read) {
      return read_;
    } else if (type == AccessType::Write) {
      return write_;
    } else {
      return false;
    }
  }
 private:
  u8* ptr_;
  bool read_;
  bool write_;
};

class CallbackOnWriteMemoryDevice : public MemoryDevice {
 public:
  CallbackOnWriteMemoryDevice(u8* ptr, std::function<void(u16, u8, u8)> callback) : ptr_(ptr), callback_(std::move(callback)) {}

  u8 Read(u16 address) override {
    return *ptr_;
  }

  void Write(u16 address, u8 value) override {
    u8 previous_value = *ptr_;
    *ptr_ = value;
    callback_(address, previous_value, value);
  }

  bool CheckAccess(u16 address, AccessType type) override {
    return true;
  }
 private:
  u8* ptr_;
  std::function<void(u16, u8, u8)> callback_;
};

template<size_t size>
class FixedMemoryDevice : public MemoryDevice {
 public:
  FixedMemoryDevice(u16 start_address, std::array<u8, size>& original, bool read, bool write) : original_(original), start_address_(start_address), read_(read), write_(write) {}

  u8 Read(u16 address) override {
    if (disabled_) {
      return 0xFF;
    }
    return original_[address - start_address_];
  }

  void Write(u16 address, u8 value) override {
    if (disabled_) {
      return;
    }
    original_[address - start_address_] = value;
  }

  bool CheckAccess(u16 address, AccessType type) override {
    u16 relative_address = address - start_address_;
    if (relative_address < 0 || relative_address > original_.size()) {
      return false;
    }
    return (type == AccessType::Read && read_) || (type == AccessType::Write && write_);
  }

  // Disable is used mainly for VRAM while PPU is accessing it
  void Disable() {
    disabled_ = true;
  }

  void Enabled() {
    disabled_ = false;
  }
 protected:
  u16 start_address_;
  std::array<u8, size>& original_;
  bool read_;
  bool write_;
  bool disabled_ = false;
};

template<size_t size>
class SwitchingMemoryDevice : public FixedMemoryDevice<size> {
 public:
  SwitchingMemoryDevice(u16 start_address, std::array<u8, size>& original, bool read, bool write) : FixedMemoryDevice<size>(start_address, original, read, write) {}

  void Switch(std::array<u8, size>& array) {
    FixedMemoryDevice<size>::original_ = array;
  }
};

class MemoryBus {
 public:
  MemoryBus();

  u8 Read(u16 address);
  void Write(u16 address, u8 value);

  u16 ReadWord(u16 address);
  void WriteWord(u16 address, u16 value);

  void panic_on_invalid_access(bool enabled) { panic_on_invalid_access_ = enabled; }
  bool panic_on_invalid_access() const { return panic_on_invalid_access_; }

  void AddDevice(u16 address, MemoryDevice* device, bool lock = false);
  void AddDevice(u16 start_address, u16 end_address, MemoryDevice* device, bool lock = false);
  void PopFrontDevice(u16 address);
  void PopFrontDevice(u16 start_address, u16 end_address);

  MemoryDevice* SelectDevice(u16 address);
 private:
  bool panic_on_invalid_access_ = true;

  std::array<std::deque<MemoryDevice*>, 0x10000> map_;
  std::array<bool, 0x10000> lock_map_;
};