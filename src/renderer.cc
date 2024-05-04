#include "renderer.h"

#include <GL/glew.h>

class TextureOGL : public Texture {
 public:
  TextureOGL(s32 width, s32 height) : width_(width), height_(height) {
    data_ = std::vector<u8>(width * height * 4, 100);

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

  void Draw(s32 x, s32 y) override {

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