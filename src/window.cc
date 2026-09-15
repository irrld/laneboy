#include "window.h"

#include <filesystem>

#include <GLFW/glfw3.h>
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

static void glfw_error_callback(int error, const char* description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Font size in logical points, before the display scale is applied.
constexpr float kFontSize = 13.0f;

// Scale of the display the window opens on. The font atlas is rasterized at
// this density so text stays sharp instead of assuming a fixed 2x display.
static float ContentScale() {
  GLFWmonitor* monitor = glfwGetPrimaryMonitor();
  if (!monitor) {
    return 1.0f;
  }
  float x_scale = 1.0f;
  float y_scale = 1.0f;
  glfwGetMonitorContentScale(monitor, &x_scale, &y_scale);
  return x_scale > 0.0f ? x_scale : 1.0f;
}
// todo maybe extract imgui stuff to another class

class WindowGLFW : public Window {
 public:
  explicit WindowGLFW(const WindowProperties& properties) {
    glfwSetErrorCallback(glfw_error_callback);
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, GL_TRUE);

    window_ = glfwCreateWindow(properties.width, properties.height, properties.title.c_str(), nullptr, nullptr);
    //glfwSetWindowSizeLimits(window_, properties.width, properties.height, properties.width, properties.height);
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(0);

    int framebuffer_width, framebuffer_height;
    glfwGetFramebufferSize(window_, &framebuffer_width, &framebuffer_height);

    glfwShowWindow(window_);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      auto& style = ImGui::GetStyle();
      style.WindowRounding = 0.0f;
      style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // The GLFW backend rewrites DisplaySize and DisplayFramebufferScale every
    // frame, so ImGui geometry stays in logical units and only the font atlas
    // has to account for the display scale.
    float scale = ContentScale();
    std::filesystem::path font = AssetPath("fonts/RobotoMono-Medium.ttf");
    if (!font.empty()) {
      io.Fonts->AddFontFromFileTTF(font.string().c_str(), kFontSize * scale);
    } else {
      // Packaged builds do not ship the font, so fall back rather than assert.
      ImFontConfig config;
      config.SizePixels = kFontSize * scale;
      io.Fonts->AddFontDefault(&config);
    }
    io.FontGlobalScale = 1.0f / scale;

    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init();

    glViewport(0, 0, framebuffer_width, framebuffer_height);
  }

  ~WindowGLFW() {
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window_);
    glfwTerminate();
  };

  void BeginFrame() override {
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
  }

  void EndFrame() override {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    // Update and Render additional Platform Windows
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      GLFWwindow* backup_current_context = glfwGetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(backup_current_context);
    }
    ImGui::EndFrame();
    glfwSwapBuffers(window_);
  }

  void SetSize(s32 width, s32 height) override {

  }

  void SetTitle(const std::string& title) override {

  }

  bool ShouldClose() override {
    return glfwWindowShouldClose(window_);
  }

 private:
  GLFWwindow* window_;
};

std::unique_ptr<Window> CreateWindow(const WindowProperties& properties) {
  return std::make_unique<WindowGLFW>(properties);
}