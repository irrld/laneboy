#pragma once

#include "cartridge.h"
#include "cpu.h"
#include "ppu.h"
#include "util.h"
#include "window.h"
#include "renderer.h"


class Emulator {
 public:
  using clock = std::chrono::steady_clock;

  Emulator();

  void Start();

  bool LoadCartridge(const std::string& file_path);

 private:
  bool running_ = false;
  std::unique_ptr<Window> window_;
  std::unique_ptr<Renderer> renderer_;
  std::unique_ptr<Texture> output_;
  std::unique_ptr<TextureWrapper> output_wrapper_;

  std::unique_ptr<MemoryBus> bus_;
  std::unique_ptr<CPU> cpu_;
  std::unique_ptr<PPU> ppu_;


  std::chrono::time_point<clock> next_cpu_cycle_;
  std::chrono::time_point<clock> next_ppu_cycle_;

 private:
  void StepEmulation();
  void Run();

  void Update();
  void Render();
};