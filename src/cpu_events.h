
#include "event.h"

class CPUModeChangeEvent : public Event {
 public:
  explicit CPUModeChangeEvent(CPUMode mode) : mode_(mode) {}

  CPUMode mode() { return mode_; }

  EVENT_CLASS_TYPE(EventTypeCPUModeChange)
  EVENT_CLASS_CATEGORY(EventCategoryCPU)
 private:
  CPUMode mode_;
};

class LCDControlChangeEvent : public Event {
 public:
  explicit LCDControlChangeEvent(LCDC lcdc, LCDC previous_lcdc) : lcdc_(lcdc), previous_lcdc_(previous_lcdc) {}

  LCDC lcdc() { return lcdc_; }
  LCDC previous_lcdc() { return previous_lcdc_; }

  EVENT_CLASS_TYPE(EventTypeLCDControlChange)
  EVENT_CLASS_CATEGORY(EventCategoryCPU)
 private:
  LCDC lcdc_;
  LCDC previous_lcdc_;
};

