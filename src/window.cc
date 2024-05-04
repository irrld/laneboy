#include "window.h"

#include <GLFW/glfw3.h>
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

static void glfw_error_callback(int error, const char* description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
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
    glfwSetWindowSizeLimits(window_, properties.width, properties.height, properties.width, properties.height);
    glfwMakeContextCurrent(window_);

    int width, height;
    int framebuffer_width, framebuffer_height;
    glfwGetWindowSize(window_, &width, &height);
    glfwGetFramebufferSize(window_, &framebuffer_width, &framebuffer_height);

    glfwShowWindow(window_);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    io.DisplaySize = ImVec2((float)width, (float)height);
    io.DisplayFramebufferScale = ImVec2((float)framebuffer_width / width, (float)framebuffer_height / height);
    float scaleFactor = 0.5f;
    io.FontGlobalScale = scaleFactor;
    io.Fonts->AddFontFromFileTTF("fonts/RobotoMono-Medium.ttf", 25);

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