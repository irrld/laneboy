#pragma once

#include "cartridge.h"
#include "cpu.h"
#include "ppu.h"
#include "util.h"
#include "window.h"
#include "renderer.h"
#include "debug.h"
#include <thread>


class Emulator {
 public:
  using clock = std::chrono::high_resolution_clock;

  Emulator();

  void Start();

  bool LoadCartridge(const std::string& file_path);

 private:
  bool running_ = false;
  std::string cartridge_path_;
  std::unique_ptr<EventBus> event_bus_;
  std::unique_ptr<Window> window_;
  std::unique_ptr<Renderer> renderer_;
  std::unique_ptr<Texture> output_;
  std::unique_ptr<Texture> vram_output_;
  std::unique_ptr<TextureWrapper> output_wrapper_;
  std::unique_ptr<TextureWrapper> vram_output_wrapper_;
  bool update_image_ = false;

  std::unique_ptr<MemoryBus> bus_;
  std::unique_ptr<CPU> cpu_;
  std::unique_ptr<PPU> ppu_;


  std::chrono::time_point<clock> next_cpu_cycle_;
  std::chrono::time_point<clock> next_window_cycle_;

  std::unique_ptr<std::thread> emulator_thread_;

 private:
  void StepEmulation();
  void Run();

  void Update();
  void Render();

#ifdef ENABLE_DEBUGGER
  void RenderDebugger();
  void RenderRegisters();
  void RenderMemoryViewer();
  void RenderCallStack();
  void RenderVRAM();

  std::vector<std::string> GetRegisters();
#endif
};
