#pragma once

#include "util.h"
#include "imgui.h"

struct WindowProperties {
  std::string title;
  s32 width;
  s32 height;
};

class Window {
 public:
  virtual void BeginFrame() = 0;
  virtual void EndFrame() = 0;

  virtual void SetSize(s32 width, s32 height) = 0;
  virtual void SetTitle(const std::string& title) = 0;

  virtual bool ShouldClose() = 0;
};

std::unique_ptr<Window> CreateWindow(const WindowProperties& properties);