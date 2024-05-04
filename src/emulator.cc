#include "emulator.h"

Emulator::Emulator() {
  ppu_ = nullptr;
  cpu_ = nullptr;
  bus_ = nullptr;
}

void Emulator::Start() {
  // startup window and prepare opengl
  window_ = CreateWindow({
      "Laneboy",
      640,
      480
  });
  renderer_ = CreateRenderer();
  output_ = renderer_->CreateTexture(160, 144);
  output_wrapper_ = std::make_unique<TextureWrapper>(*output_);
  Run();
}

bool Emulator::LoadCartridge(const std::string& file_path) {
  ppu_ = nullptr;
  cpu_ = nullptr;
  bus_ = nullptr;

  std::unique_ptr<Cartridge> cartridge = std::make_unique<Cartridge>("rom/zelda.gb");
  if (!cartridge->is_valid()) {
    std::cout << "cartridge is not valid, shutting down!" << std::endl;
    return false;
  }
  next_cpu_cycle_ = clock::now();
  next_ppu_cycle_ = clock::now();

  bus_ = std::make_unique<MemoryBus>();
  cpu_ = std::make_unique<CPU>(*bus_);

  std::vector<u8> boot = LoadBin("rom/cgb_boot.bin");
  // first load the rom, this will have priority over the cartridge memory
  cpu_->LoadBootRom(boot.data(), boot.size());
  // load cartridge
  cpu_->LoadCartridge(std::move(cartridge));

  ppu_ = std::make_unique<PPU>(*cpu_, *bus_);

  cpu_->running_ = true;
  return true;
}

void Emulator::StepEmulation() {
  if (!cpu_ || !cpu_->running_) {
    return;
  }

  auto now = clock::now();
  if (now >= next_cpu_cycle_) {
    if (!cpu_->halted_) {
      cpu_->HandleInterrupts();
      cpu_->Step();
    }
    next_cpu_cycle_ += std::chrono::nanoseconds(static_cast<long>(1000000000 / cpu_->clock_speed_));
  }
  if (now >= next_ppu_cycle_) {
    cpu_->Step();
    next_ppu_cycle_ += std::chrono::nanoseconds(static_cast<long>(1000000000 / ppu_->clock_speed_));
  }
}

void Emulator::Run() {
  running_ = true;
  while (running_ && !window_->ShouldClose()) {
    StepEmulation();
    Update();
    window_->BeginFrame();
    Render();
    window_->EndFrame();
  }
}

void Emulator::Update() {
}

void Emulator::Render() {
  ImGui::SetNextWindowPos(ImVec2(0,0));
  ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
  ImGui::Begin("Main", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus);
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Open ROM...")) {

      }
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }

  // fit the image to the window
  ImVec2 avail = ImGui::GetContentRegionAvail();
  float aspect_ratio = static_cast<float>(output_->width()) / output_->height();
  float scaled_width = avail.x;
  float scaled_height = avail.x / aspect_ratio;
  if (scaled_height > avail.y) {
    scaled_height = avail.y;
    scaled_width = avail.y * aspect_ratio;
  }
  output_->DrawImGui(scaled_width, scaled_height);
  ImGui::End();
  ImGui::PopStyleVar();
}