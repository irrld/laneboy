#include "renderer.h"

#include <GL/glew.h>

Colori ColorFromHex(u32 value) {
  return {(u8)((value >> 16) & 0xFF), (u8)((value >> 8) & 0xFF), (u8)(value & 0xFF)};
}

class TextureOGL : public Texture {
 public:
  TextureOGL(s32 width, s32 height) : width_(width), height_(height) {
    data_ = std::vector<u8>(width * height * 4);
    std::fill(data_.begin(), data_.end(), 0);

    glGenTextures(1, &id_);
    glBindTexture(GL_TEXTURE_2D, id_);

    // todo min/mag filter options?
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    UploadData();
  }

  ~TextureOGL() {
    glDeleteTextures(1, &id_);
  }

  s32 width() const override { return width_; }

  s32 height() const override {
    return height_;
  }

  void Draw(s32 x, s32 y, s32 width, s32 height) override {
  }

  void DrawImGui(s32 width, s32 height) override {
    float window_width = ImGui::GetWindowWidth();
    float indent = (window_width - width) / 2;
    ImGui::SetCursorPosX(indent);

    ImVec2 size(width, height);
    ImGui::Image((void*)(intptr_t)id_, size, ImVec2(0, 0), ImVec2(1, 1));
  }

 private:
  GLuint id_;
  std::vector<u8> data_;
  s32 width_, height_;

 private:
  std::vector<u8>& data_internal() override {
    return data_;
  }

  void UploadData() override {
    glBindTexture(GL_TEXTURE_2D, id_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_.data());
  }
};

TextureWrapper::TextureWrapper(Texture& texture) : texture_(texture), data_(texture.data_internal()) {

}

void TextureWrapper::SetPixel(s32 x, s32 y, Colori color) {
  if (color.a == 0) {
    return;
  }
  u64 index = y * texture_.width() * 4 + x * 4; // *4 because every pixel has 4 components
  data_[index] = color.r;
  data_[index + 1] = color.g;
  data_[index + 2] = color.b;
  data_[index + 3] = color.a;
  changed_ = true;
}

void TextureWrapper::Fill(Colori color) {
  for (int x = 0; x < texture_.width(); ++x) {
    for (int y = 0; y < texture_.height(); ++y) {
      SetPixel(x, y, color);
    }
  }
}

void TextureWrapper::Update() {
  if (changed_) {
    texture_.UploadData();
    changed_ = false;
  }
}

class RendererOGL : public Renderer {
 public:
  void ClearColor(Colorf color) override {
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
  }

  std::unique_ptr<Texture> CreateTexture(s32 width, s32 height) override {
    return std::make_unique<TextureOGL>(width, height);
  }
};


std::unique_ptr<Renderer> CreateRenderer() {
  return std::make_unique<RendererOGL>();
}