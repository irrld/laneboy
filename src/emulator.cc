#include "emulator.h"
#include "debug.h"
#include <unordered_set>
#include "tinyfiledialogs.h"

Emulator::Emulator() {
  event_bus_ = nullptr;
  window_ = nullptr;
  renderer_ = nullptr;
  output_ = nullptr;
  output_wrapper_ = nullptr;
  bus_ = nullptr;
  cpu_ = nullptr;
  ppu_ = nullptr;
}

void Emulator::Start() {
  event_bus_ = std::make_unique<EventBus>();
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
  if (cpu_) {
    cpu_->running_ = false;
  }
  RESET_DEBUGGER();
  if (emulator_thread_ && emulator_thread_->joinable()) {
    emulator_thread_->join();
  }
  emulator_thread_ = nullptr;
  ppu_ = nullptr;
  cpu_ = nullptr;
  bus_ = nullptr;

  cartridge_path_ = file_path;
  std::unique_ptr<Cartridge> cartridge = std::make_unique<Cartridge>(file_path);
  if (!cartridge->is_valid()) {
    std::cout << "cartridge is not valid, shutting down!" << std::endl;
    return false;
  }
  next_cpu_cycle_ = clock::now();

  bus_ = std::make_unique<MemoryBus>();
  cpu_ = std::make_unique<CPU>(*event_bus_, *bus_);

  // first load the rom, this will have priority over the cartridge memory
  cpu_->LoadBootRom(LoadBin("rom/fast_boot.bin"));

  ppu_ = std::make_unique<PPU>(*event_bus_, *cpu_, *bus_, *output_wrapper_);

  // load cartridge
  cpu_->LoadCartridge(std::move(cartridge));

  INIT_DEBUGGER(*bus_);
  cpu_->running_ = true;

  emulator_thread_ = std::make_unique<std::thread>([this]() {
    while (cpu_ && cpu_->running_) {
      StepEmulation();
    }
  });
  return true;
}

void Emulator::StepEmulation() {
  if (!cpu_ || !cpu_->running_) {
    return;
  }

  auto now = clock::now();
  if (now >= next_cpu_cycle_) {
    if (!cpu_->halted_) {
      cpu_->Step();
      cpu_->ProcessDMA();
      cpu_->HandleInterrupts();
    } else {
      cpu_->cycles_consumed_ = 4;
    }
    cpu_->UpdateTimers(cpu_->cycles_consumed_);
    // todo, PPU it should run at /2 cycle count in double speed mode!
    for (int i = 0; i < cpu_->cycles_consumed_; ++i) {
      ppu_->Step();
      if (ppu_->frame_complete_) {
        update_image_ = true;
      }
    }
    next_cpu_cycle_ += std::chrono::nanoseconds(static_cast<long>(1'000'000'000 / cpu_->clock_speed_)) * cpu_->cycles_consumed_ * 4;
  }
  auto end = clock::now();
  next_cpu_cycle_ -= std::chrono::duration_cast<std::chrono::microseconds>(end - now);
}

void Emulator::Run() {
  running_ = true;
  while (running_ && !window_->ShouldClose()) {
    //auto now = clock::now();
    //if (now >= next_window_cycle_) {
    Update();
    window_->BeginFrame();
    Render();
    window_->EndFrame();
    //next_window_cycle_ += std::chrono::nanoseconds(static_cast<long>(1'000'000'000 / 60.0));
    //}
  }
  // make sure we cleanup the CPU and the emulator before emulator object gets destructed
  if (cpu_) {
    cpu_->running_ = false;
  }
  if (emulator_thread_ && emulator_thread_->joinable()) {
    emulator_thread_->join();
  }
  emulator_thread_ = nullptr;
}

void Emulator::Update() {
  if (update_image_ && ppu_ && ppu_->frames_rendered_ != 0) {
    output_wrapper_->Update();
    update_image_ = false;
  }
}

void Emulator::Render() {
  renderer_->ClearColor({255, 255, 255, 255});
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->WorkPos);
  ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::Begin("My Fullscreen Window", nullptr,
               ImGuiWindowFlags_NoDecoration |
                   ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoSavedSettings |
                   ImGuiWindowFlags_NoBringToFrontOnFocus);
  ImGui::PopStyleVar();
  ImGui::PopStyleVar();

  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Load ROM...")) {
        // Define the file filter pattern
        char const *file_filter_patterns[2] = { "*.gb", "*.rom" };

        // Open a file dialog and get the file name
        char const *file_name = tinyfd_openFileDialog(
            "Open Game Boy ROM",
            "",
            2,
            file_filter_patterns,
            NULL,
            0
        );

        if (file_name) {
          LoadCartridge(file_name);
        }
      }
      if (!cartridge_path_.empty() && ImGui::MenuItem("Restart")) {
        LoadCartridge(cartridge_path_);
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
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

#ifdef ENABLE_DEBUGGER
  if (cpu_) {
    RenderDebugger();
    RenderRegisters();
    RenderMemoryViewer();
  }
#endif
}

#ifdef ENABLE_DEBUGGER

struct InstructionEntry {
  std::string label = "";
  u16 address = 0;
};

void Emulator::RenderDebugger() {
  static u16 previous_address = 0;
  u16 current_address = Debugger::GetCurrentInstruction();
  bool moved = current_address != previous_address;
  previous_address = current_address;
  if (!ImGui::Begin("Debugger")) {
    ImGui::End();
    return;
  }

  // Use a Child window to hold the scrollable list of instructions

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
  ImGui::BeginChild("InstructionList", ImVec2(0,  -ImGui::GetTextLineHeightWithSpacing() * 2), false, ImGuiWindowFlags_HorizontalScrollbar);
  ImGui::PopStyleVar();


  static std::vector<InstructionEntry> names;
  static std::unordered_map<u16, size_t> index_map;
  if (Debugger::CheckInstructionsChangedAndClear()) {
    names.clear();
    index_map.clear();
    for (u32 i = 0; i <= 0xFFFF; i++) {
      u16 address = (u16) i;
      int length = Debugger::GetInstructionLengthAt(address);
      if (length == 0) {
        // Skip rendering this address because it's part of a multi-byte instruction
        continue;
      }

      std::string instruction = Debugger::GetInstructionAt(address);
      if (instruction.empty()) {
        continue; // Skip empty instructions
      }
      std::string label = ToHex(address) + ": " + instruction;
      names.push_back({label, address});
      index_map.insert(std::make_pair(address, names.size() - 1));
    }
  }

  int start_index = 0;
  int end_index = 0;
  u16 first_address = 0;

  // Render only the visible items
  ImGuiListClipper clipper;
  clipper.Begin(names.size());
  while (clipper.Step()) {
    start_index = clipper.DisplayStart;
    end_index = clipper.DisplayEnd;
    for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; index++) {
      const InstructionEntry& entry = names[index];
      if (first_address == 0) {
        first_address = entry.address;
      }

      bool isBreakpoint = Debugger::HasBreakpoint(entry.address);
      bool isCurrent = entry.address == current_address;
      char label[256];
      snprintf(label, sizeof(label), "%s", entry.label.c_str());

      // Styling for current or breakpoint lines
      if (isCurrent) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));  // Green for the current instruction
      } else if (isBreakpoint) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));  // Red for breakpoints
      }

      if (ImGui::Selectable(label, isCurrent, ImGuiSelectableFlags_AllowDoubleClick)) {
        if (ImGui::IsMouseDoubleClicked(0)) {
          if (isBreakpoint) {
            Debugger::SetBreakpoint(entry.address, false);
          } else {
            Debugger::SetBreakpoint(entry.address, true);
          }
        }
      }

      if (isCurrent || isBreakpoint) {
        ImGui::PopStyleColor();
      }
    }
  }

  static double next_scroll = -1;
  u32 current_index = index_map[current_address];
  // Scroll to the current instruction if it's not in the visible range
  if (moved && (current_index <= start_index || current_index >= end_index)) {
    double target_scroll = (double)current_index * 16.9997; // wtf??
    next_scroll = target_scroll;
  }

  if (next_scroll >= 0) {
    ImGui::SetScrollY(next_scroll);
    next_scroll = -1;
  }

  ImGui::EndChild();

  if (ImGui::Button("Continue")) {
    Debugger::Continue();
  }
  ImGui::SameLine();
  if (ImGui::Button("Step")) {
    Debugger::Step();
  }
  ImGui::SameLine();
  if (ImGui::Button("Pause")) {
    Debugger::Pause();
  }

  ImGui::SameLine();
  char data[7];
  snprintf(data, sizeof(data), "%04X", first_address);
  if (ImGui::InputText("Jump To Address", data, sizeof(data), ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue)) {
    u16 in;
    if (sscanf(data, "%x", &in) == 1) {
      u32 target_index = index_map[in];
      next_scroll = (double)target_index * 16.9997;
    }
  }

  ImGui::End();
}

std::vector<std::string> Emulator::GetRegisters() {
  std::vector<std::string> registers = {
      {"A: " + ToHex(cpu_->registers_.a)},
      {"BC: " + ToHex(cpu_->registers_.Get(ArithmeticTarget::BC))},
      {"DE: " + ToHex(cpu_->registers_.Get(ArithmeticTarget::DE))},
      {"HL: " + ToHex(cpu_->registers_.Get(ArithmeticTarget::HL))},
      {" "},
      {"FLAGS: " + ToBinary(cpu_->registers_.flags.v)},
      {" zero: " + BoolToStr(cpu_->registers_.flags.f.zero)},
      {" carry: " + BoolToStr(cpu_->registers_.flags.f.carry)},
      {" half carry: " + BoolToStr(cpu_->registers_.flags.f.half_carry)},
      {" subtract: " + BoolToStr(cpu_->registers_.flags.f.subtract)},
      {" "},
      {"SP: " + ToHex(cpu_->registers_.Get(ArithmeticTarget::SP))},
      {"PC: " + ToHex(cpu_->registers_.pc)},
      {" "},
      {"TAC: " + ToHex(cpu_->tac_)},
      {"DIV: " + ToHex(cpu_->div_)},
      {"TIMA: " + ToHex(cpu_->tima_)},
      {"TMA: " + ToHex(cpu_->tma_)},
  };
  return registers;
}

void Emulator::RenderRegisters() {
  auto registers = GetRegisters();

  if (!ImGui::Begin("Registers")) {
    ImGui::End();
    return;
  }

  // Display each register with its value
  for (const auto& name : registers) {
    if (!name.empty()) {
      ImGui::Text("%s", name.c_str());
    } else {
      ImGui::Text(" ");
    }
  }

  ImGui::End();
}

void Emulator::RenderMemoryViewer() {
  if (!ImGui::Begin("Memory Viewer")) {
    ImGui::End();
    return;
  }

  ImGui::BeginChild("MemoryScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
  u16 last_changed_address = Debugger::GetPreviousWrittenAddress();
  u8 last_changed_value = Debugger::GetPreviousWrittenValue(); // previous value

  // Setup a table with 17 columns: address and 16 bytes per line
  if (ImGui::BeginTable("MemoryTable", 17, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY)) {

    // Populate the table with memory data
    for (int i = 0; i <= 0xFFFF; i += 16) {
      ImGui::TableNextRow();

      // Address column
      ImGui::TableSetColumnIndex(0);
      ImGui::Text("0x%04X", i);

      // Bytes columns
      for (int j = 0; j < 16; ++j) {
        int address = i + j;
        ImGui::TableSetColumnIndex(j + 1);

        // Color the last changed address differently
        if (address == last_changed_address) {
          ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        }

        if (!bus_->CheckAccess(address, kMemoryAccessRead)) {
          ImGui::Selectable("--", false, ImGuiSelectableFlags_Disabled);
          continue;
        }
        u8 value = bus_->Read(address);

        char data[5];
        snprintf(data, sizeof(data), "%02X", value);
        std::string label = "##Byte" + std::to_string(address);
        std::string str = ToHex(value, false);
        if (ImGui::Selectable(str.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_DontClosePopups)) {
          if (ImGui::IsMouseDoubleClicked(0)) {
            ImGui::OpenPopup(label.c_str());
          }
        }
        if (address == last_changed_address) {
          ImGui::PopStyleColor();
        }

        if (ImGui::BeginPopup(label.c_str())) {
          ImGui::Text("Edit Value");
          if (ImGui::InputText("##edit", data, sizeof(data), ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue)) {
            u8 in;
            if (sscanf(data, "%x", &in) == 1) {
              bus_->Write(address, in);
            }
            ImGui::CloseCurrentPopup();
          }
          ImGui::EndPopup();
        }

        // Tooltip for old values
        if (address == last_changed_address) {
          if (ImGui::IsItemHovered()) {
            auto* device = bus_->SelectDevice(address);
            std::string device_name = device != nullptr ? GetNameFromPointer(device) : "None";
            ImGui::SetTooltip("Address: 0x%04X\nBinary: %s\nOld Value: 0x%02X\nDevice: %s", address, ToBinary(value).c_str(), last_changed_value, device_name.c_str());
          }
        } else {
          if (ImGui::IsItemHovered()) {
            auto* device = bus_->SelectDevice(address);
            std::string device_name = device != nullptr ? GetNameFromPointer(device) : "None";
            ImGui::SetTooltip("Address: 0x%04X\nBinary: %s\nDevice: %s", address, ToBinary(value).c_str(), device_name.c_str());
          }
        }
      }
    }
    ImGui::EndTable();
  }

  ImGui::EndChild();

  ImGui::End();
}
#endif
