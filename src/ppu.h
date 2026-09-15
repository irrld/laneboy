#pragma once

#include <array>

#include "cpu.h"
#include "cpu_events.h"
#include "event.h"
#include "memory.h"
#include "util.h"
#include "renderer.h"

constexpr int kScreenWidth = 160;
constexpr int kScreenHeight = 144;

// A scanline is 456 dots: 80 of OAM scan, then a variable draw, then HBlank
// for whatever is left. Lines 144-153 are VBlank.
constexpr int kDotsPerLine = 456;
constexpr int kOAMScanDots = 80;
constexpr int kLinesPerFrame = 154;

// The fetcher needs this many dots to fill the FIFO before the first pixel of a
// line reaches the LCD, which is what makes mode 3 at least 172 dots long.
constexpr int kDrawStartupDots = 12;

constexpr int kObjectCount = 40;
constexpr int kMaxObjectsPerLine = 10;

// Background and window pixels waiting to be pushed to the LCD. The fetcher
// fills it a tile at a time, the LCD drains one pixel per dot.
class PixelFIFO {
 public:
  void Push(Pixel pixel) {
    assert(size_ < kCapacity);
    pixels_[(head_ + size_) % kCapacity] = pixel;
    size_++;
  }

  Pixel Pop() {
    assert(size_ > 0);
    Pixel pixel = pixels_[head_];
    head_ = (head_ + 1) % kCapacity;
    size_--;
    return pixel;
  }

  void Reset() {
    head_ = 0;
    size_ = 0;
  }

  bool Has() const {
    return size_ > 0;
  }

  u8 Size() const {
    return size_;
  }

 private:
  static constexpr u8 kCapacity = 16;

  u8 head_ = 0;
  u8 size_ = 0;
  Pixel pixels_[kCapacity]{};
};

class PPU {
 public:
  PPU(EventBus& event_bus, CPU& cpu, MemoryBus& bus, TextureWrapper& output_wrapper);
  ~PPU();

  PPU(const PPU&) = delete;

  void Step();
  void SetClockSpeed(u32 clock_speed);

  void OnEvent(Event& event);
  bool OnLCDControlChange(LCDControlChangeEvent& event);

  void ResetFrame();

  // for final rendering
  void FillImage(Colori color);
  void SetPixel(u16 x, u16 y, Colori color);
  void UpdateImage();

  u8 FetchTileId(u8 x, u8 y, bool high_tilemap);
  std::array<Pixel, 8> FetchTile(u16 tile_index, u8 y, bool is_background);

  Colori GetColor(u8 index, ColorMode mode);

 public:
  EventBus& event_bus_;
  // we currently access memory directly from CPU, instead
  // there should be a different bus where CPU could lock it easily to prevent PPU's access
  // same would go for the RAMBUS, PPU could also lock the ram access to for certain areas.
  CPU& cpu_;
  MemoryBus& bus_;
  TextureWrapper& output_wrapper_;
  u32 clock_speed_;

  // Dots elapsed in the current scanline, 0 to kDotsPerLine - 1.
  u16 line_dot_ = 0;
  bool frame_complete_ = false;
  u32 frames_rendered_ = 0;

  // OAM byte offsets of the objects visible on this line, in priority order.
  u16 current_line_objects_[kMaxObjectsPerLine];
  u8 current_line_object_num_ = 0;

  // Fetcher position within the line, including the pixels discarded for the
  // fine horizontal scroll.
  u8 lx_ = 0;
  u8 mod_scx_ = 0;
  u8 draw_delay_ = 0;

  // The window has its own line counter: it only advances on lines where the
  // window was actually drawn, not with LY.
  u8 window_line_ = 0;
  bool in_window_ = false;
  bool window_drawn_this_line_ = false;
  u8 window_tile_x_ = 0;

  PixelFIFO bg_fifo_;

  bool was_enabled_ = true;

 private:
  void SetMode(PPUMode mode);
  void UpdateLYC();

  void BeginLine(u8 ly);
  void BeginDraw();

  void StepOAMScan();
  void StepDraw();

  void SortLineObjects();
  bool WindowVisible() const;
  void FetchBackgroundTile();
  Pixel MixObject(Pixel background, u8 screen_x);

  void DrawPixel(Pixel pixel, u8 x, u8 y);
};
