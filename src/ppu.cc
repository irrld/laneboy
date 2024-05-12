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
    cycles_consumed_ = 1;
    vblank_lines_ = 0;
    return;
  }
  cycles_consumed_ = 1;
  if (cpu_.lcds_.bits.ppu_mode == kPPUModeVBlank) {
    cpu_.SendInterrupt(kInterruptTypeVBlank);
    if (vblank_lines_ <= 0 && frame_complete_) {
      ResetFrame();
    }
    if (hblank_wait_ <= 0) {
      cpu_.ly_++;
      vblank_lines_--;
      hblank_wait_ = 460;
    }
  }

  // switch modes
  if (cpu_.lcds_.bits.ppu_mode == kPPUModeOAMScan && oam_scan_index_ == 10) {
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
  // update
  u8 ly = cpu_.ly_;
  u8 lyc = cpu_.lyc_;
  cpu_.lcds_.bits.lyc_ly_compare = ly == lyc;
  if (ly == lyc && cpu_.lcds_.bits.lyc_int_select) {
    cpu_.SendInterrupt(kInterruptTypeLCDStat);
  }

  // process
  if (cpu_.lcds_.bits.ppu_mode == kPPUModeOAMScan) {
    if (oam_scan_index_ == 0) {
      sub_scx_ = cpu_.scx_ % 8;
    }
    u8 pos_y = cpu_.oam_[oam_scan_index_ * 4] - 16;
    if (pos_y >= ly && pos_y < ly + 16 && current_line_object_num_ < 10) {
      // save the object and increase the object num
      current_line_objects_[current_line_object_num_++] = oam_scan_index_;
    }
    oam_scan_index_++;
    cycles_consumed_ += 2;
  } else if (cpu_.lcds_.bits.ppu_mode == kPPUModeDraw && !draw_done_) {
    /*if (current_line_object_num_ > 0) {
      for (int i = 0; i < current_line_object_num_; ++i) {
        u8 pos_y = cpu_.oam_[current_line_objects_[i]];
        u8 pos_x = cpu_.oam_[current_line_objects_[i] + 1];
        if (pos_x != lx_) {
          continue;
        }
        u8 tile_index = cpu_.oam_[current_line_objects_[i] + 2];
        ObjectAttributeFlags flags{cpu_.oam_[current_line_objects_[i] + 3]};
        TileData data = FetchTile(tile_index);
        for (int j = 0; j < 8; ++j) {
          int k = flags.bits.x_flip ? 7 - j : j;
          Pixel pixel = data.pixels[(ly + pos_y + 16) % 8][k];
          oam_fifo_.Push(pixel);
        }
      }
    }*/
    if (lx_ % 8 == 0) {
      u8 tile_id = FetchTileIdFromBackground(lx_, ly, cpu_.scx_, cpu_.scy_);
      TileData data = FetchTile(tile_id, true);
      for (int i = 0; i < 8; ++i) {
        Pixel pixel = data.pixels[(ly + (cpu_.scy_ % 8)) % 8][(i + sub_scx_) % 8];
        bg_fifo_.Push(pixel);
      }
    }
    u8 ox = 0;
    while (bg_fifo_.Has()) {
      DrawPixel(bg_fifo_.Pop(), lx_ - ox + 7, ly, true);
      ox++;
    }
    ox = 0;
    while (oam_fifo_.Has()) {
      DrawPixel(oam_fifo_.Pop(), lx_ - ox + 7, ly, false);
      ox++;
    }
    draw_took_ += cycles_consumed_;
    lx_++;
    if (lx_ >= 160) {
      draw_done_ = true;
    }
  }

  hblank_wait_ -= std::min(cycles_consumed_, hblank_wait_);
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

TileData PPU::GetTile(u8 index) {
  TileData data{};
  for (int i = 0; i < 8; ++i) {
    u8 msb = cpu_.vram_0_[index + i * 2];
    u8 lsb = cpu_.vram_0_[index + i * 2 + 1];
    for (int j = 0; j < 8; ++j) {
      u8 mask = (1 << j);
      bool b1 = msb & mask;
      bool b2 = lsb & mask;
      u8 color_id = b2 << 1 | b1;
      data.pixels[i][8 - j - 1].color = color_id;
      data.pixels[i][8 - j - 1].bg_priority = 0; // todo?
    }
  }
  return data;
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

void PPU::DrawPixel(Pixel pixel, u8 x, u8 y, bool is_background) {
  if (pixel.color == 0 && is_background) {
    output_wrapper_.SetPixel(x, y, {155, 188, 15, 255});
  } else if (pixel.color == 1) {
    output_wrapper_.SetPixel(x, y, {139, 172, 15, 255});
  } else if (pixel.color == 2) {
    output_wrapper_.SetPixel(x, y, {48, 98, 48, 255});
  } else if (pixel.color == 3) {
    output_wrapper_.SetPixel(x, y, {15, 56, 15, 255});
  }
}

u8 PPU::FetchTileIdFromBackground(u8 x, u8 y, u8 sx, u8 sy) {
  // calcula(te the pixel coordinates within the full 256x256 background map
  u8 bg_x = (x + sx) % 256;
  u8 bg_y = (y + sy) % 256;

  // calculate tile map coordinates
  u32 tilemap_x = bg_x / 8;
  u32 tilemap_y = bg_y / 8;

  u16 offset = 0x9800;
  if (cpu_.lcdc_.bits.bg_tilemap_area) { // todo window area check
    offset = 0x9C00;
  }
  u16 tilemap_address = offset + tilemap_y * 32 + tilemap_x;

  // fetch the tile number from the tile map
  return bus_.Read(tilemap_address);
}

TileData PPU::FetchTile(u8 tile_index, bool is_background) {
  // calculate the tile address
  // assuming that the tile data table starts at VRAM address 0x8000
  u16 offset = 0x8000;
  if (is_background) {
    if (!cpu_.lcdc_.bits.bg_window_tile_area) {
      offset = 0x9000;
      if (tile_index < 128) {
        offset = 0x8800;
      }
    }
  }
  u16 tile_address = offset + tile_index * 16;

  // fetch the tile data from VRAM
  TileData fetched_tile{};
  for (int i = 0; i < 8; i++) {
    u8 msb = bus_.Read(tile_address + 2 * i); // GetTileDataHigh
    u8 lsb = bus_.Read(tile_address + 2 * i + 1); // GetTileDataLow
    for (int j = 0; j < 8; j++) {
      bool b1 = (msb >> j) & 0x1;
      bool b2 = (lsb >> j) & 0x1;
      u8 color_id = (b2 << 1) | b1;

      fetched_tile.pixels[i][7 - j].color = color_id;
      fetched_tile.pixels[i][7 - j].bg_priority = 1;
    }
  }
  return fetched_tile;

}
