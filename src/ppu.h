#pragma once

#include "cpu.h"
#include "cpu_events.h"
#include "event.h"
#include "memory.h"
#include "util.h"
#include "renderer.h"

class PixelFIFO {
 public:
  void Push(Pixel pixel) {
    assert(a_ < 16);
    pixels[a_] = pixel;
    a_++;
  }

  Pixel Pop() {
    assert(a_ > 0);
    Pixel pixel = pixels[a_ - 1];
    a_--;
    return pixel;
  }

  void Reset() {
    a_ = 0;
  }

  bool Has() {
    return a_ > 0;
  }

  u8 Size() {
    return a_;
  }
 private:
  u8 a_ = 0;
  Pixel pixels[16];
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

  u8 FetchTileIdFromBackground(u8 x, u8 y);
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
  u16 vblank_lines_ = 0;
  u16 hblank_wait_ = 0;
  u16 draw_took_ = 0;
  bool draw_done_ = false;
  bool frame_complete_ = false;
  u32 frames_rendered_ = 0;
  u16 current_line_objects_[10];
  u8 current_line_object_num_ = 0;
  u8 oam_scan_index_;

  u8 lx_;

  u8 mod_scx_;

  PixelFIFO bg_fifo_;
  PixelFIFO oam_fifo_;

  bool was_enabled_ = true;

 private:
  void SetMode(PPUMode mode);

  void DrawPixel(Pixel pixel, u8 x, u8 y);
};