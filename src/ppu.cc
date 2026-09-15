#include "ppu.h"

#include <algorithm>

PPU::PPU(EventBus& event_bus, CPU& cpu, MemoryBus& bus, TextureWrapper& output_wrapper) : event_bus_(event_bus), cpu_(cpu), bus_(bus), output_wrapper_(output_wrapper) {
  SetClockSpeed(BASE_PPU_CLOCK_SPEED);
  event_bus_.Subscribe(BIND_FN(OnEvent));
}

PPU::~PPU() {
  // todo unsub from event bus
}

void PPU::Step() {
  if (was_enabled_ != cpu_.lcdc_.bits.lcd_enable) {
    was_enabled_ = cpu_.lcdc_.bits.lcd_enable;
    frames_rendered_ = 0;
    output_wrapper_.Fill({255, 255, 255, 255});
    if (was_enabled_) {
      ResetFrame();
    } else {
      // Turning the LCD off parks it at the start of line 0 in HBlank.
      cpu_.ly_ = 0;
      line_dot_ = 0;
      frame_complete_ = false;
      SetMode(kPPUModeHBlank);
    }
    std::cout << "display enable changed: " << BoolToStr(was_enabled_) << std::endl;
  }
  if (!cpu_.lcdc_.bits.lcd_enable) {
    return;
  }

  switch (cpu_.lcds_.bits.ppu_mode) {
    case kPPUModeOAMScan:
      StepOAMScan();
      break;
    case kPPUModeDraw:
      StepDraw();
      break;
    case kPPUModeHBlank:
    case kPPUModeVBlank:
      break;
  }

  line_dot_++;
  if (cpu_.lcds_.bits.ppu_mode == kPPUModeOAMScan && line_dot_ >= kOAMScanDots) {
    BeginDraw();
  }
  if (line_dot_ >= kDotsPerLine) {
    BeginLine(cpu_.ly_ + 1);
  }
}

void PPU::SetClockSpeed(u32 clock_speed) {
  clock_speed_ = clock_speed;
}

void PPU::OnEvent(Event& event) {
  EventDispatcher dispatcher{event};
  dispatcher.Dispatch<LCDControlChangeEvent>(BIND_FN(OnLCDControlChange));
}

bool PPU::OnLCDControlChange(LCDControlChangeEvent& event) {
  return false;
}

void PPU::FillImage(Colori color) {
  output_wrapper_.Fill(color);
}

void PPU::SetPixel(u16 x, u16 y, Colori color) {
  output_wrapper_.SetPixel(x, y, color);
}

void PPU::UpdateImage() {
  output_wrapper_.Update();
}

void PPU::ResetFrame() {
  line_dot_ = 0;
  window_line_ = 0;
  frame_complete_ = false;
  cpu_.ly_ = 0;
  UpdateLYC();
  current_line_object_num_ = 0;
  SetMode(kPPUModeOAMScan);
}

void PPU::BeginLine(u8 ly) {
  line_dot_ = 0;
  if (ly >= kLinesPerFrame) {
    ly = 0;
    window_line_ = 0;
    frames_rendered_++;
    frame_complete_ = false;
  }
  cpu_.ly_ = ly;
  UpdateLYC();

  if (ly < kScreenHeight) {
    current_line_object_num_ = 0;
    SetMode(kPPUModeOAMScan);
  } else if (ly == kScreenHeight) {
    frame_complete_ = true;
    SetMode(kPPUModeVBlank);
  }
}

void PPU::BeginDraw() {
  lx_ = 0;
  mod_scx_ = cpu_.scx_ % 8;
  draw_delay_ = kDrawStartupDots;
  in_window_ = false;
  window_drawn_this_line_ = false;
  window_tile_x_ = 0;
  bg_fifo_.Reset();
  SortLineObjects();
  SetMode(kPPUModeDraw);
}

void PPU::StepOAMScan() {
  // Two dots per object, 40 objects, 80 dots.
  if (line_dot_ % 2 != 0) {
    return;
  }
  u8 index = line_dot_ / 2;
  if (index >= kObjectCount || current_line_object_num_ >= kMaxObjectsPerLine) {
    return;
  }

  u16 base = index * 4;
  s16 pos_y = static_cast<s16>(cpu_.oam_[base]) - 16;
  s16 height = cpu_.lcdc_.bits.obj_size ? 16 : 8;
  if (cpu_.ly_ >= pos_y && cpu_.ly_ < pos_y + height) {
    current_line_objects_[current_line_object_num_++] = base;
  }
}

void PPU::SortLineObjects() {
  // On DMG the object with the smaller X wins, ties go to the lower OAM index.
  // The scan collected them in OAM order, so a stable sort by X is enough.
  std::stable_sort(current_line_objects_,
                   current_line_objects_ + current_line_object_num_,
                   [this](u16 a, u16 b) {
                     return cpu_.oam_[a + 1] < cpu_.oam_[b + 1];
                   });
}

void PPU::StepDraw() {
  if (draw_delay_ > 0) {
    draw_delay_--;
    return;
  }

  s16 screen_x = static_cast<s16>(lx_) - mod_scx_;

  // The window takes over the fetcher for the rest of the line as soon as the
  // LCD reaches WX-7, and restarts it from the first window tile.
  if (!in_window_ && WindowVisible() && screen_x >= static_cast<s16>(cpu_.wx_) - 7) {
    in_window_ = true;
    window_drawn_this_line_ = true;
    window_tile_x_ = 0;
    bg_fifo_.Reset();
  }

  if (!bg_fifo_.Has()) {
    FetchBackgroundTile();
  }

  Pixel background = bg_fifo_.Pop();
  if (screen_x >= 0 && screen_x < kScreenWidth) {
    DrawPixel(MixObject(background, static_cast<u8>(screen_x)), static_cast<u8>(screen_x), cpu_.ly_);
  }

  lx_++;
  if (screen_x + 1 >= kScreenWidth) {
    if (window_drawn_this_line_) {
      window_line_++;
    }
    SetMode(kPPUModeHBlank);
  }
}

bool PPU::WindowVisible() const {
  return cpu_.lcdc_.bits.window_enable && cpu_.ly_ >= cpu_.wy_ && cpu_.wx_ <= 166;
}

void PPU::FetchBackgroundTile() {
  // LCDC.0 clear blanks the background and the window on DMG.
  if (!cpu_.lcdc_.bits.bg_window) {
    for (int i = 0; i < 8; i++) {
      bg_fifo_.Push(Pixel{0, kColorModeBackground});
    }
    return;
  }

  u8 tile_x;
  u8 tile_y;
  u8 row;
  bool high_tilemap;
  if (in_window_) {
    tile_x = window_tile_x_++;
    tile_y = window_line_ / 8;
    row = window_line_ % 8;
    high_tilemap = cpu_.lcdc_.bits.window_tilemap_area;
  } else {
    // Both coordinates wrap inside the 256x256 tile map.
    u8 y = cpu_.ly_ + cpu_.scy_;
    tile_x = (cpu_.scx_ / 8) + (lx_ / 8);
    tile_y = y / 8;
    row = y % 8;
    high_tilemap = cpu_.lcdc_.bits.bg_tilemap_area;
  }

  u8 tile_id = FetchTileId(tile_x, tile_y, high_tilemap);
  std::array<Pixel, 8> data = FetchTile(tile_id, row, true);
  for (Pixel pixel : data) {
    pixel.mode = kColorModeBackground;
    bg_fifo_.Push(pixel);
  }
}

Pixel PPU::MixObject(Pixel background, u8 screen_x) {
  if (!cpu_.lcdc_.bits.obj_enable) {
    return background;
  }

  s16 height = cpu_.lcdc_.bits.obj_size ? 16 : 8;
  for (int i = 0; i < current_line_object_num_; i++) {
    u16 base = current_line_objects_[i];
    s16 pos_x = static_cast<s16>(cpu_.oam_[base + 1]) - 8;
    if (screen_x < pos_x || screen_x >= pos_x + 8) {
      continue;
    }
    s16 pos_y = static_cast<s16>(cpu_.oam_[base]) - 16;
    ObjectAttributeFlags flags{cpu_.oam_[base + 3]};

    u8 row = static_cast<u8>(cpu_.ly_ - pos_y);
    if (flags.bits.y_flip) {
      row = height - 1 - row;
    }
    u8 tile_index = cpu_.oam_[base + 2];
    if (height == 16) {
      // The low bit of the tile index picks the half, so it is ignored.
      tile_index = (tile_index & 0xFE) | (row >= 8 ? 1 : 0);
      row %= 8;
    }

    u8 column = static_cast<u8>(screen_x - pos_x);
    if (flags.bits.x_flip) {
      column = 7 - column;
    }
    std::array<Pixel, 8> data = FetchTile(tile_index, row, false);
    u8 color = data[column].color;
    if (color == 0) {
      // Transparent, a lower priority object may still show through here.
      continue;
    }
    // The highest priority object covering this pixel has won the slot, so the
    // only question left is whether the background draws over it. With LCDC.0
    // clear on DMG the object always wins.
    if (flags.bits.priority && background.color != 0 && cpu_.lcdc_.bits.bg_window) {
      return background;
    }
    return Pixel{color, flags.bits.dmg_palette ? kColorModeObjectPalette1 : kColorModeObjectPalette0};
  }
  return background;
}

void PPU::UpdateLYC() {
  cpu_.lcds_.bits.lyc_ly_compare = cpu_.ly_ == cpu_.lyc_;
  if (cpu_.lcds_.bits.lyc_ly_compare && cpu_.lcds_.bits.lyc_int_select) {
    cpu_.SendInterrupt(kInterruptTypeLCDStat);
  }
}

void PPU::SetMode(PPUMode mode) {
  if (cpu_.lcds_.bits.ppu_mode == mode) {
    return;
  }
  cpu_.lcds_.bits.ppu_mode = mode;
  if ((mode == kPPUModeOAMScan && cpu_.lcds_.bits.mode_2_int_select) ||
      (mode == kPPUModeVBlank && cpu_.lcds_.bits.mode_1_int_select) ||
      (mode == kPPUModeHBlank && cpu_.lcds_.bits.mode_0_int_select)) {
    cpu_.SendInterrupt(kInterruptTypeLCDStat);
  }
  if (mode == kPPUModeVBlank) {
    cpu_.SendInterrupt(kInterruptTypeVBlank);
  }
}

void PPU::DrawPixel(Pixel pixel, u8 x, u8 y) {
  Colori color = GetColor(pixel.color, pixel.mode);
  output_wrapper_.SetPixel(x, y, color);
}

u8 PPU::FetchTileId(u8 x, u8 y, bool high_tilemap) {
  u16 offset = high_tilemap ? 0x9C00 : 0x9800;
  return bus_.Read(offset + (x % 32) + ((y % 32) * 32));
}

std::array<Pixel, 8> PPU::FetchTile(u16 tile_index, u8 y, bool is_background) {
  u16 offset = 0x8000;
  if (is_background) {
    if (!cpu_.lcdc_.bits.bg_window_tile_area) {
      offset = 0x9000;
      s8 signed_index = static_cast<s8>(tile_index);
      offset += (s16)signed_index * 0x10;
      tile_index = 0;
    }
  }
  u16 tile_address = offset + (u16)tile_index * 0x10;

  // fetch the tile data from VRAM, low bit plane first
  std::array<Pixel, 8> fetched_tile{};
  u8 low = bus_.Read(tile_address + (2 * y));
  u8 high = bus_.Read(tile_address + (2 * y) + 1);
  for (int j = 0; j < 8; j++) {
    bool b1 = (low >> j) & 0x1;
    bool b2 = (high >> j) & 0x1;
    u8 color_id = (b2 << 1) | b1;

    fetched_tile[7 - j].color = color_id;
  }

  return fetched_tile;
}

Colori PPU::GetColor(u8 index, ColorMode mode) {
  u8 paletted_index;
  if (mode == kColorModeObjectPalette0) {
    paletted_index = (cpu_.obp0_ >> (index * 2)) & 0b11;
  } else if (mode == kColorModeObjectPalette1) {
    paletted_index = (cpu_.obp1_ >> (index * 2)) & 0b11;
  } else { // background
    paletted_index = (cpu_.bgp_ >> (index * 2)) & 0b11;
  }
  const u32 colors[]{0xFFFFFF, 0xD3D3D3, 0xA9A9A9, 0x000000};
  // Object transparency is already resolved by the time a pixel gets here.
  Colori color = ColorFromHex(colors[paletted_index]);
  color.a = 255;
  return color;
}
