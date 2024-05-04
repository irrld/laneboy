
#pragma once

#include "util.h"
#include <functional>

enum EventType {
  EventTypeLast
};

#define BIT(x) (1 << x)

enum EventCategory {
  EventCategoryMemory = BIT(0)
};

#undef BIT

class Event {
 public:
  virtual ~Event() = default;

  bool handled_ = false;

  virtual const char* GetEventName() const = 0;
  virtual EventType GetEventType() const = 0;
  virtual int GetCategoryFlags() const = 0;

  bool IsInCategory(EventCategory category) {
    return GetCategoryFlags() & category;
  }
};

using EventCallbackFn = std::function<void(Event&)>;

class EventDispatcher {
 public:
  EventDispatcher(Event& event) : event_(event) {}

  // F will be deduced by the compiler
  template <typename T, typename F>
  bool Dispatch(const F& func) {
    if (event_.GetEventType() == T::GetStaticType()) {
      event_.handled_ |= func(static_cast<T&>(event_));
      return true;
    }
    return false;
  }

 private:
  Event& event_;
};

#define EVENT_CLASS_TYPE(type)                   \
  static EventType GetStaticType() {                  \
    return EventType::type;                           \
  }                                                   \
  virtual EventType GetEventType() const override {   \
    return GetStaticType();                           \
  }                                                   \
  virtual const char* GetEventName() const override { \
    return #type;                                     \
  }

#define EVENT_CLASS_CATEGORY(category)       \
  virtual int GetCategoryFlags() const override { \
    return category;                              \
  }
