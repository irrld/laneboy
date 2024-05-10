#pragma once

#include "util.h"
#include <stack>
#include <utility>

// Virtual Memory System


enum MemoryAccess : u8 {
  kMemoryAccessNone = 0b00,
  kMemoryAccessRead = 0b01,
  kMemoryAccessWrite = 0b10,
  kMemoryAccessBoth = 0b11
};

// Overloaded bitwise OR:
inline MemoryAccess operator|(MemoryAccess a, MemoryAccess b) {
  return static_cast<MemoryAccess>(static_cast<u8>(a) | static_cast<u8>(b));
}

// Overloaded bitwise AND:
inline MemoryAccess operator&(MemoryAccess a, MemoryAccess b) {
  return static_cast<MemoryAccess>(static_cast<u8>(a) & static_cast<u8>(b));
}


class MemoryDevice {
 public:
  MemoryDevice(MemoryAccess access) : access_(access) {}
  MemoryDevice(const MemoryDevice&) = delete;

  virtual u8 Read(u16 address) = 0;

  virtual void Write(u16 address, u8 value) = 0;

  virtual bool CheckAccess(u16 address, MemoryAccess type) {
    return (access_ & type) == type;
  }

  virtual void DisableAccess(MemoryAccess access) {
    access_ = static_cast<MemoryAccess>(access_ & ~access);
  }

  virtual void EnableAccess(MemoryAccess access) {
    access_ = static_cast<MemoryAccess>(access_ | access);
  }

  virtual void OnFailedWrite(u16 address, u8 value) {
  }
 protected:
  MemoryAccess access_;
};

template <typename T>
concept OneByteType = sizeof(T) == 1;

template <size_t T>
concept One = T == 1;

template <size_t T>
concept NotOne = T != 1;

template<size_t size>
class FixedArrayMemoryDevice : public MemoryDevice {
 public:
  FixedArrayMemoryDevice(u16 start_address, std::array<u8, size>* original, MemoryAccess access) : MemoryDevice(access), original_(original), start_address_(start_address) {}

  u8 Read(u16 address) override {
    return (*original_)[address - start_address_];
  }

  void Write(u16 address, u8 value) override {
    (*original_)[address - start_address_] = value;
  }

  bool CheckAccess(u16 address, MemoryAccess type) override {
    u16 relative_address = address - start_address_;
    if (relative_address < 0 || relative_address >= size) {
      return false;
    }
    return (access_ & type) == type;
  }

 protected:
  u16 start_address_;
  std::array<u8, size>* original_;
};

template<size_t size>
class FixedArrayWithHandlerMemoryDevice : public FixedArrayMemoryDevice<size> {
 public:
  FixedArrayWithHandlerMemoryDevice(u16 start_address, std::array<u8, size>* original, std::function<u8(u16, u8, u8)> handler, MemoryAccess access)
      : super(start_address, original, access), handler_(std::move(handler)) {}

  void Write(u16 address, u8 value) override {
    u16 relative_address = address - super::start_address_;
    u8 old_value = (*super::original_)[relative_address];
    (*super::original_)[relative_address] = handler_(address, old_value, value);
  }

 private:
  using super = FixedArrayMemoryDevice<size>;
  std::function<u8(u16, u8, u8)> handler_;
};

template<size_t size>
class SwitchingArrayMemoryDevice : public FixedArrayMemoryDevice<size> {
 public:
  SwitchingArrayMemoryDevice(u16 start_address, std::array<u8, size>* original, MemoryAccess access) : super(start_address, original, access) {}

  void Switch(std::array<u8, size>* array) {
    super::original_ = array;
  }

 private:
  using super = FixedArrayMemoryDevice<size>;
};

template<size_t size>
class SwitchingArrayWithHandlerMemoryDevice : public SwitchingArrayMemoryDevice<size> {
 public:
  SwitchingArrayWithHandlerMemoryDevice(u16 start_address, std::array<u8, size>* original, std::function<u8(u16, u8, u8, bool)> handler, MemoryAccess access)
      : super(start_address, original, access), handler_(std::move(handler)) {}

  void Write(u16 address, u8 value) override {
    u16 relative_address = address - super::start_address_;
    u8 old_value = (*super::original_)[relative_address];
    (*super::original_)[relative_address] = handler_(address, old_value, value, false);
  }

  void OnFailedWrite(u16 address, u8 value) override {
    u16 relative_address = address - super::start_address_;
    u8 old_value = (*super::original_)[relative_address];
    handler_(address, old_value, value, true);
  }

 private:
  using super = SwitchingArrayMemoryDevice<size>;
  std::function<u8(u16, u8, u8, bool)> handler_;
};

template<size_t size, OneByteType Type>
class FixedPointerMemoryDevice : public MemoryDevice {
 public:
  template<size_t Dummy = 0>
  requires One<size>
  FixedPointerMemoryDevice(Type* original, MemoryAccess access) : MemoryDevice(access), original_(original), start_address_(0) {}

  template<size_t Dummy = 0>
  requires NotOne<size>
  FixedPointerMemoryDevice(u16 start_address, Type* original, MemoryAccess access) : MemoryDevice(access), original_(original), start_address_(start_address) {}

  u8 Read(u16 address) override {
    if constexpr (size == 1) {
      return *reinterpret_cast<u8*>(original_);
    } else {
      return reinterpret_cast<u8*>(original_)[address - start_address_];
    }
  }

  void Write(u16 address, u8 value) override {
    if constexpr (size == 1) {
      *reinterpret_cast<u8*>(original_) = value;
    } else {
      reinterpret_cast<u8*>(original_)[address - start_address_] = value;
    }
  }

  bool CheckAccess(u16 address, MemoryAccess type) override {
    if constexpr (size > 1) {
      u16 relative_address = address - start_address_;
      if (relative_address < 0 || relative_address >= size) {
        return false;
      }
    }
    return (access_ & type) == type;
  }

 protected:
  u16 start_address_;
  Type* original_;
};

template<size_t size, OneByteType Type>
class FixedPointerWithHandlerMemoryDevice : public FixedPointerMemoryDevice<size, Type> {
 public:
  template<size_t Dummy = 0>
  requires One<size>
  FixedPointerWithHandlerMemoryDevice(Type* original, std::function<Type(u16, Type, Type, bool)> handler, MemoryAccess access)
      : super(original, access), handler_(std::move(handler)) {}

  template<size_t Dummy = 0>
  requires NotOne<size>
  FixedPointerWithHandlerMemoryDevice(u16 start_address, Type* original, std::function<Type(u16, Type, Type, bool)> handler, MemoryAccess access)
      : super(start_address, original, access), handler_(std::move(handler)) {}

  void Write(u16 address, u8 value) override {
    if constexpr (size == 1) {
      Type old_value = *super::original_;
      *super::original_ = handler_(address, old_value, Type(value), false);
    } else {
      u16 relative_address = address - super::start_address_;
      Type old_value = super::original_[relative_address];
      super::original_[relative_address] = handler_(address, old_value, Type(value), false);
    }
  }

  void OnFailedWrite(u16 address, u8 value) override {
    if constexpr (size == 1) {
      Type old_value = *super::original_;
      handler_(address, old_value, Type(value), true);
    } else {
      u16 relative_address = address - super::start_address_;
      Type old_value = super::original_[relative_address];
      handler_(address, old_value, Type(value), true);
    }
  }

 private:
  using super = FixedPointerMemoryDevice<size, Type>;
  std::function<Type(u16, Type, Type, bool)> handler_;
};

template<size_t size, OneByteType Type>
class SwitchingPointerMemoryDevice : public FixedPointerMemoryDevice<size, Type> {
 public:
  template<size_t Dummy = 0>
    requires One<size>
  SwitchingPointerMemoryDevice(u8* original, MemoryAccess access) : super(original, access) {}

  template<size_t Dummy = 0>
    requires NotOne<size>
  SwitchingPointerMemoryDevice(u16 start_address, u8* original, MemoryAccess access) : super(start_address, original, access) {}

  void Switch(u8* data) {
    super::original_ = data;
  }
 private:
  using super = FixedPointerMemoryDevice<size, Type>;
};

template<size_t size, OneByteType Type>
class SwitchingPointerWithHandlerMemoryDevice : public SwitchingPointerMemoryDevice<size, Type> {
 public:
  template<size_t Dummy = 0>
    requires One<size>
  SwitchingPointerWithHandlerMemoryDevice(Type* original, std::function<Type(u16, Type, Type)> handler, MemoryAccess access)
      : super(original, access), handler_(std::move(handler)) {}

  template<size_t Dummy = 0>
    requires NotOne<size>
  SwitchingPointerWithHandlerMemoryDevice(u16 start_address, Type* original, std::function<Type(u16, Type, Type)> handler, MemoryAccess access)
      : super(start_address, original, access), handler_(std::move(handler)) {}

  void Write(u16 address, u8 value) override {
    if constexpr (size == 1) {
      Type old_value = *super::original_;
      *super::original_ = handler_(address, old_value, Type(value));
    } else {
      u16 relative_address = address - super::start_address_;
      Type old_value = super::original_[relative_address];
      super::original_[relative_address] = handler_(address, old_value, Type(value));
    }
  }

 private:
  using super = SwitchingPointerMemoryDevice<size, Type>;
  std::function<Type(u16, Type, Type)> handler_;
};

class TesterMemoryDevice : public MemoryDevice {
 public:
  TesterMemoryDevice(u16 start_address, u8* original, u32 size, std::function<u8(u16, u8, bool, MemoryAccess)> callback, MemoryAccess access)
      : MemoryDevice(access), original_(original), size_(size), start_address_(start_address), callback_(std::move(callback)) {}

  u8 Read(u16 address) override {
    u16 actual_address = address - start_address_;
    u8 value = original_[actual_address];
    value = callback_(address, value, false, kMemoryAccessRead);
    return value;
  }

  void Write(u16 address, u8 value) override {
    u16 actual_address = address - start_address_;
    value = callback_(address, value, false, kMemoryAccessWrite);
    original_[actual_address] = value;
  }

  void OnFailedWrite(u16 address, u8 value) override {
    callback_(address, value, true, kMemoryAccessWrite);
  }

  bool CheckAccess(u16 address, MemoryAccess type) override {
    u16 relative_address = address - start_address_;
    if (relative_address < 0 || relative_address >= size_) {
      return false;
    }
    return (access_ & type) == type;
  }

 protected:
  u16 start_address_;
  u8* original_;
  u32 size_;
  std::function<u8(u16, u8, bool, MemoryAccess)> callback_;
};

class MemoryBus {
 public:
  MemoryBus();
  MemoryBus(const MemoryBus&) = delete;

  void Reset();

  void WriteWord(u16 address, u16 value);
  void Write(u16 address, u8 value);
  void Write(u16 address, u16 value) = delete;

  u8 Read(u16 address);
  u16 ReadWord(u16 address);

  bool CheckAccess(u16 address, MemoryAccess access);

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