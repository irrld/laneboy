
#pragma once

#include "util.h"
#include "imgui.h"

template<typename Number>
struct Color {
  Number r,g,b,a;
};

using Colorf = Color<float>;

class Texture {
 public:
  virtual s32 width() const = 0;
  virtual s32 height() const = 0;

  const std::vector<u8>& data() {
    return data_internal();
  };

  virtual void Draw(s32 x, s32 y) = 0;
  virtual void DrawImGui(s32 width, s32 height) = 0;

 private:
  friend class TextureWrapper;

  virtual std::vector<u8>& data_internal() = 0;
  virtual void UploadData() = 0;
};

/**
 * An util class to edit the texture easily, it will store the initial
 */
class TextureWrapper {
 public:
  TextureWrapper(Texture& texture);


 private:
  Texture& texture_;
  std::vector<u8>& data_;
};


class Renderer {
 public:
  virtual void ClearColor(Colorf color) = 0;

  virtual std::unique_ptr<Texture> CreateTexture(s32 width, s32 height) = 0;

};

std::unique_ptr<Renderer> CreateRenderer();