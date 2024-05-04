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
  CallbackOnWriteMemoryDevice(u8* ptr, std::function<void(u16)> callback) : ptr_(ptr), callback_(std::move(callback)) {}

  u8 Read(u16 address) override {
    return *ptr_;
  }

  void Write(u16 address, u8 value) override {
    *ptr_ = value;
  }

  bool CheckAccess(u16 address, AccessType type) override {
    return true;
  }
 private:
  u8* ptr_;
  std::function<void(u16)> callback_;
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