#include "ppu.h"

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
    std::cout << "display enable changed: " << BoolToStr(was_enabled_) << std::endl;
  }
  if (!cpu_.lcdc_.bits.lcd_enable) {
    vblank_lines_ = 0;
    return;
  }

  if (cpu_.lcds_.bits.ppu_mode == kPPUModeVBlank) {
    if (vblank_lines_ <= 0 && frame_complete_) {
      ResetFrame();
    }
    if (hblank_wait_ <= 0) {
      cpu_.ly_++;
      cpu_.lcds_.bits.lyc_ly_compare = cpu_.ly_ == cpu_.lyc_;
      if (cpu_.lcds_.bits.lyc_ly_compare && cpu_.lcds_.bits.lyc_int_select) {
        cpu_.SendInterrupt(kInterruptTypeLCDStat);
      }
      vblank_lines_--;
      hblank_wait_ = 460;
    }
  }

  // switch modes
  if (cpu_.lcds_.bits.ppu_mode == kPPUModeOAMScan && oam_scan_index_ >= 0x9F) {
    // switch to draw mode
    SetMode(kPPUModeDraw);
    oam_scan_index_ = 0;
    draw_done_ = false;
    draw_took_ = 0;
    bg_fifo_.Reset();
    oam_fifo_.Reset();
  }
  if (cpu_.lcds_.bits.ppu_mode == kPPUModeDraw && draw_done_) {
    SetMode(kPPUModeHBlank);
    hblank_wait_ = 376 - draw_took_;
    draw_done_ = false;
    draw_took_ = 0;
  }
  if (cpu_.lcds_.bits.ppu_mode == kPPUModeHBlank && hblank_wait_ <= 0) {
    hblank_wait_ = 0;
    cpu_.ly_++;
    lx_ = 0;
    if (cpu_.ly_ >= 144) {
      SetMode(kPPUModeVBlank);
      vblank_lines_ = 10;
      hblank_wait_ = 456;
      frame_complete_ = true;
    } else {
      SetMode(kPPUModeOAMScan);
      oam_scan_index_ = 0;
      current_line_object_num_ = 0;
    }
  }

  // process
  u8 ly = cpu_.ly_;
  if (cpu_.lcds_.bits.ppu_mode == kPPUModeOAMScan) {
    if (oam_scan_index_ == 0) {
      mod_scx_ = cpu_.scx_ % 8;
    }
    if (oam_scan_index_ % 4 == 0) {
      u8 pos_y = cpu_.oam_[oam_scan_index_] - 16;
      if (pos_y >= ly && pos_y < ly + 16 && current_line_object_num_ < 10) {
        // save the object and increase the object num
        current_line_objects_[current_line_object_num_++] = oam_scan_index_;
      }
    }
    oam_scan_index_++;
  } else if (cpu_.lcds_.bits.ppu_mode == kPPUModeDraw && !draw_done_) {
    /*if (current_line_object_num_ > 0 && cpu_.lcdc_.bits.obj_enable) {
      for (int i = 0; i < current_line_object_num_; ++i) {
        u8 pos_y = cpu_.oam_[current_line_objects_[i]] - 16;
        u8 pos_x = cpu_.oam_[current_line_objects_[i] + 1] - 8;
        if (pos_x != lx_) {
          continue;
        }
        u8 tile_y = pos_y % 16;
        u8 tile_index = cpu_.oam_[current_line_objects_[i] + 2];
        if (cpu_.lcdc_.bits.obj_size) {
          if (tile_y < 8) {
            tile_index = tile_index & 0xFE;
          } else {
            tile_index = tile_index | 0x01;
            tile_y = tile_y % 8;
          }
        }
        ObjectAttributeFlags flags{cpu_.oam_[current_line_objects_[i] + 3]};
        std::array<Pixel, 8> data = FetchTile(tile_index, tile_y, false);
        for (int j = 7; j >= 0; j--) {
          int k = flags.bits.x_flip ? 7 - j : j;
          Pixel pixel = data[k];
          pixel.mode = !flags.bits.dmg_palette ? kColorModeObjectPalette0 : kColorModeObjectPalette1;
          oam_fifo_.Push(pixel);
        }
      }
    }*/
    if (lx_ % 8 == 0) { // fetch tile
      u8 div_scy = cpu_.scy_ / 8;
      u8 div_scx = cpu_.scx_ / 8;
      u8 mod_scy = cpu_.scy_ % 8;
      u8 tile_y = div_scy + (ly / 8);
      u8 ry = ly % 8;
      if (ry + mod_scy >= 8) {
        tile_y++;
        ry = (ry + mod_scy) % 8;
      } else {
        ry += mod_scy;
      }

      u8 tile_id = FetchTileIdFromBackground(div_scx + (lx_ / 8), tile_y);
      std::array<Pixel, 8> data = FetchTile(tile_id, ry, true);
      for (int x = 7; x >= 0; x--) {
        Pixel pixel = data[x];
        pixel.mode = kColorModeBackground;
        bg_fifo_.Push(pixel); // push tile
      }
    }

    if (lx_ >= mod_scx_) { // pixel is inside the screen, draw
      DrawPixel(bg_fifo_.Pop(), lx_ - mod_scx_, ly);
    } else { // pixel is outside the screen, discard
      bg_fifo_.Pop();
    }

    if (oam_fifo_.Has()) {
      DrawPixel(oam_fifo_.Pop(), lx_, ly);
    }

    draw_took_ += 1;
    lx_++;
    if (lx_ >= 160 + mod_scx_) {
      draw_done_ = true;
    }
  }

  hblank_wait_ -= std::min((u16)1, hblank_wait_);
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
  if (frame_complete_) {
    frames_rendered_++;
  }
  //output_wrapper_.Fill({255, 255, 255, 255});
  SetMode(kPPUModeOAMScan);
  cpu_.ly_ = 0;
  current_line_object_num_ = 0;
  oam_scan_index_ = 0;
  vblank_lines_ = 4560; // 10 lines, 456 dots each
  draw_done_ = false;
  draw_took_ = 0;
  frame_complete_ = false;
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

u8 PPU::FetchTileIdFromBackground(u8 x, u8 y) {
  u16 offset = 0x9800;
  if (cpu_.lcdc_.bits.bg_tilemap_area) { // todo window area check
    offset = 0x9C00;
  }
  u16 tilemap_address = offset + (x % 32) + ((y % 32) * 32);

  // fetch the tile number from the tile map
  u8 value = bus_.Read(tilemap_address);
  return value;
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

  // fetch the tile data from VRAM
  std::array<Pixel, 8> fetched_tile{};
  u8 msb = bus_.Read(tile_address + (2 * y)); // GetTileDataHigh
  u8 lsb = bus_.Read(tile_address + (2 * y) + 1); // GetTileDataLow
  for (int j = 0; j < 8; j++) {
    bool b1 = (msb >> j) & 0x1;
    bool b2 = (lsb >> j) & 0x1;
    u8 color_id = (b2 << 1) | b1;

    fetched_tile[7 - j].color = color_id;
  }

  return fetched_tile;
}

Colori PPU::GetColor(u8 index, ColorMode mode) {
  u8 paletted_index;
  if (mode == kColorModeObjectPalette0) {
    paletted_index = (cpu_.obp0_ >> (index * 2)) & 0b11;
    //return {255, 0, 0, 255};
  } else if (mode == kColorModeObjectPalette1) {
    paletted_index = (cpu_.obp1_ >> (index * 2)) & 0b11;
    //return {255, 0, 0, 255};
  } else { // background
    paletted_index = (cpu_.bgp_ >> (index * 2)) & 0b11;
  }
  const u32 colors[]{0xFFFFFF, 0xD3D3D3, 0xA9A9A9, 0x000000};
  Colori color = ColorFromHex(colors[paletted_index]);
  if (mode == kColorModeBackground) {
    color.a = 255;
  } else {
    color.a = 0;
  }
  return color;
}