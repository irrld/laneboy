#pragma once

#include <limits>
#include <iostream>
#include <cassert>
#include <bitset>
#include <vector>
#include <string>
#include <sstream>
#include <fmt/core.h>
#include <iomanip>
#include <typeinfo>
#include <cxxabi.h>

#define VRAM_START_ADDRESS 0x8000
#define VRAM_END_ADDRESS 0x9FFF
#define VRAM_SIZE 0x2000
#define VRAM_BANK_SELECT_ADDRESS 0xFF4F

// Joypad
#define JOYP_ADDRESS 0xFF00
// Serial I/O
#define SB_ADDRESS 0xFF01
#define SC_ADDRESS 0xFF02
// CPU MODE REGISTER
#define CPU_MODE_ADDRESS 0xFF4C
#define CPU_MODE_LOCK_ADDRESS 0xFF6C

#define KEY1_ADDRESS 0xFF4D

#define LCD_CONTROL_ADDRESS 0xFF40
#define LCD_STAT_ADDRESS 0xFF41
#define LCD_SCY_ADDRESS 0xFF42
#define LCD_SCX_ADDRESS 0xFF43
#define LCD_LY_ADDRESS 0xFF44
#define LCD_LYC_ADDRESS 0xFF45
#define DMA_ADDRESS 0xFF46
// Non-CGB mode only
#define LCD_BGP_ADDRESS 0xFF47
// Non-CGB mode only
#define LCD_OBP0_ADDRESS 0xFF48
// Non-CGB mode only
#define LCD_OBP1_ADDRESS 0xFF49
#define LCD_WY_ADDRESS 0xFF4A
#define LCD_WX_ADDRESS 0xFF4B

// left side is DMG name, right side is CGB name
// can be read outside vblank and hblank
#define LCD_BCPS_BGPI_ADDRESS 0xFF68
// cannot be read during mode 3
#define LCD_BCPD_BGPD_ADDRESS 0xFF69
#define LCD_OCPS_OBPI_ADDRESS 0xFF6A
#define LCD_OCPD_OBPD_ADDRESS 0xFF6B

#define CARTRIDGE_ROM_00_START_ADDRESS 0x0000
#define CARTRIDGE_ROM_00_END_ADDRESS 0x3FFF
#define CARTRIDGE_ROM_01_START_ADDRESS 0x4000
#define CARTRIDGE_ROM_01_END_ADDRESS 0x7FFF
#define CARTRIDGE_ROM_SIZE 0x4000

#define CARTRIDGE_RAM_START_ADDRESS 0xA000
#define CARTRIDGE_RAM_END_ADDRESS 0xBFFF
#define CARTRIDGE_RAM_SIZE 0x2000

#define WRAM_0_START_ADDRESS 0xC000
#define WRAM_0_END_ADDRESS 0xCFFF
#define WRAM_1_7_START_ADDRESS 0xD000
#define WRAM_1_7_END_ADDRESS 0xDFFF
#define WRAM_SIZE 0x1000
#define WRAM_BANK_SELECT_ADDRESS 0xFF70

#define OAM_START_ADDRESS 0xFE00
#define OAM_END_ADDRESS 0xFE9F
#define OAM_SIZE 0x0100

#define HRAM_START_ADDRESS 0xFF80
#define HRAM_END_ADDRESS 0xFFFE
#define HRAM_SIZE 0x0080

#define AUDIO_START_ADDRESS 0xFF10
#define AUDIO_END_ADDRESS 0xFF26
#define AUDIO_SIZE 0x0017

#define WAVE_PATTERN_START_ADDRESS 0xFF30
#define WAVE_PATTERN_END_ADDRESS 0xFF3F
#define WAVE_PATTERN_SIZE 0x0010

#define BOOT_UNMAP_ADDRESS 0xFF50
#define INTERRUPT_ENABLE_ADDRESS 0xFFFF
#define INTERRUPT_FLAG_ADDRESS 0xFF0F

#define SB_ADDRESS 0xFF01
#define SC_ADDRESS 0xFF02

// Timer stuff
#define DIV_ADDRESS 0xFF04
#define TIMA_ADDRESS 0xFF05
#define TMA_ADDRESS 0xFF06
#define TAC_ADDRESS 0xFF07

#define BASE_CPU_CLOCK_SPEED 4194304
#define DOUBLE_CPU_CLOCK_SPEED BASE_CPU_CLOCK_SPEED * 2
#define BASE_PPU_CLOCK_SPEED BASE_CPU_CLOCK_SPEED

#define BIND_FN(fn)                                    \
  [this](auto&&... args) -> decltype(auto) {                \
    return this->fn(std::forward<decltype(args)>(args)...); \
  }

#define BIND_GLOBAL_FN(fn)                       \
  [](auto&&... args) -> decltype(auto) {              \
    return fn(std::forward<decltype(args)>(args)...); \
  }


using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;

std::vector<u8> LoadBin(const std::string& path);

inline std::string BoolToStr(bool b) {
  return b ? "true" : "false";
}

inline s8 AsSigned(u8 val) {
  // here the static cast is not required but here to silence the warnings
  if (val & (1 << 7)) { // If the high bit is set
    // Compute two's complement and return
    return static_cast<s8>(-(static_cast<s8>((~val & 0xFF) + 1)));
  }
  // If the high bit is not set
  return static_cast<s8>(val);
}

template<typename Number, int size = sizeof(Number) * 8>
inline std::string ToBinary(Number v) {
  std::stringstream stream;
  std::bitset<size> x(v);
  stream << "0b" << x;
  return stream.str();
}

template<typename Number, int size = sizeof(Number) * 2>
inline std::string ToHex(Number v, bool prefix = true) {
  std::stringstream stream;
  if (!std::is_unsigned<Number>() && v < 0) {
    v = -v;
    stream << "-";
  }
  if (prefix) {
    stream << "0x";
  }
  stream << std::setw(size)
         << std::setfill('0')
         << std::hex << std::uppercase << (uint64_t) v;
  return stream.str();
}

template<typename Number>
struct AddResult {
  Number value;
  bool did_overflow;
};

//V-Blank: 0x40
//LCD STAT: 0x48
//Timer: 0x50
//Serial: 0x58
//Joypad: 0x60
enum InterruptType : u8 {
  kInterruptTypeVBlank = 0b0000'0001,
  kInterruptTypeLCDStat = 0b0000'0010,
  kInterruptTypeTimer = 0b0000'0100,
  kInterruptTypeSerial = 0b0000'1000,
  kInterruptTypeJoypad = 0b0001'0000,
  kInterruptMax = 6
};

enum CPUMode : u8 {
  kCPUModeDMG = 0x80,
  kCPUModeCGB = 0x04,
};

struct Joypad {
  bool a_right : 1; // 0 pressed, 1 not pressed (read-only)
  bool b_left : 1; // 0 pressed, 1 not pressed (read-only)
  bool select_up : 1; // 0 pressed, 1 not pressed (read-only)
  bool start_down : 1; // 0 pressed, 1 not pressed (read-only)
  bool select_dpad : 1; // when set, lower nibble is set to down-up-left-right buttons
  bool select_buttons : 1; // when set, lower nibble is set to start-select-b-a
};


struct Pixel {
  u8 color : 2;
  u8 palette : 3;
  u8 bg_priority : 1;
  u8 sprite : 1;
  u8 padding : 1;
};

struct TileData {
  Pixel pixels[8][8];
};

u8 ProcessTileData(u16 data, u8 index);

union LCDC {
  u8 value = 0;
  struct {
    bool bg_window : 1; // bg-window enable in DMG, bg-window priority on CGB mode
    bool obj_enable : 1; // 0 = Off, 1 = On
    bool obj_size : 1; // 0 = 8x8, 1 = 8x16
    bool bg_tilemap_area : 1; // 0 = 9800-9BFF, 1 = 9C00-9FFF
    bool bg_window_tile_area : 1; // // 0 = 8800-97FF, 1 = 8000-8FFF
    bool window_enable : 1; // 0 = Off, 1 = On
    bool window_tilemap_area : 1; // 0 = 9800-9BFF, 1 = 9C00-9FFF
    bool lcd_enable : 1; // LCD & PPU Enable, 0 = Off, 1 = On
  } bits;

  explicit operator u8() const {
    return value;
  }

  LCDC() = default;
  explicit LCDC(u8 value) : value(value) { }
};

enum PPUMode : u8 {
  kPPUModeHBlank = 0x00,
  kPPUModeVBlank = 0x01,
  kPPUModeOAMScan = 0x02,
  kPPUModeDraw = 0x03,
};

union LCDS {
  u8 value = 0;
  struct {
    PPUMode ppu_mode : 2; // read only
    bool lyc_ly_compare : 1; // read only
    bool mode_0_int_select : 1;
    bool mode_1_int_select : 1;
    bool mode_2_int_select : 1;
    bool lyc_int_select : 1;
  } bits;

  explicit operator u8() const {
    return value;
  }

  LCDS() = default;
  explicit LCDS(u8 value) : value(value) { }
};

union ObjectAttributeFlags {
  u8 value;
  struct {
    u8 cgb_palette : 3; // CGB Mode only, which OBP0-7 to use
    bool bank : 1; // CGB Mode only, 0 = use VRAM 0, 1 = use VRAM 1 to fetch tile data
    bool dmg_palette : 1; // Non CGB Mode only, 0 = OBP0, 1 = OBP1
    bool x_flip : 1; // 0 = Normal, 1 = Horizontally Flipped
    bool y_flip : 1; // 0 = Normal, 1 = Vertically Flipped
    bool priority : 1; // 0 = No, 1 = BG and Window colors 1-3 are drawn over this object
  } bits;
};

enum MonochromeColor : u8 {
  kMonochromeColorWhite = 0,
  kMonochromeColorLightGray = 1,
  kMonochromeColorDarkGray = 2,
  kMonochromeColorBlack = 3
};

union MonochromePalette {
  u8 value;
  struct {
    MonochromeColor id0 : 2;
    MonochromeColor id1 : 2;
    MonochromeColor id2 : 2;
    MonochromeColor id3 : 2;
  } colors;
};

template<typename T>
std::string GetNameFromPointer(T* ptr) {
  int status;
  std::unique_ptr<char, decltype(&std::free)> res {
      abi::__cxa_demangle(typeid(*ptr).name(), nullptr, nullptr, &status),
      std::free
  };
  return res.get();
}