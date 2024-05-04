
#include "event.h"

class MemoryReadEvent : public Event {
 public:
  MemoryReadEvent(u16 address, u8 value)
      : address_(address), value_(value) {}

  u16 address() const { return address_; }
  u8 value() const { return value_; }

  EVENT_CLASS_TYPE(EventTypeMemoryRead)
  EVENT_CLASS_CATEGORY(EventCategoryMemory)
 private:
  u16 address_;
  u8 value_;
};


class MemoryWriteEvent : public Event {
 public:
  MemoryWriteEvent(u16 address, u8 old_value, u8 value)
      : address_(address), old_value_(old_value), value_(value) {}

  u16 address() const { return address_; }
  u8 value() const { return value_; }
  u8 old_value() const { return old_value_; }

  EVENT_CLASS_TYPE(EventTypeMemoryWrite)
  EVENT_CLASS_CATEGORY(EventCategoryMemory)
 private:
  u16 address_;
  u8 old_value_;
  u8 value_;
};