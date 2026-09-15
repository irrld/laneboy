#include "file_browser.h"

#include <algorithm>
#include <cstdlib>

#include "imgui.h"

namespace {

std::string ToLower(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return value;
}

std::filesystem::path DefaultDirectory() {
#if defined(_WIN32)
  const char* home = std::getenv("USERPROFILE");
#else
  const char* home = std::getenv("HOME");
#endif
  std::error_code ec;
  if (home != nullptr) {
    std::filesystem::path path{home};
    if (std::filesystem::is_directory(path, ec)) {
      return path;
    }
  }
  std::filesystem::path current = std::filesystem::current_path(ec);
  return ec ? std::filesystem::path{"/"} : current;
}

}  // namespace

void FileBrowser::Open(std::string title, std::vector<std::string> extensions) {
  title_ = std::move(title);
  extensions_ = std::move(extensions);
  for (std::string& extension : extensions_) {
    extension = ToLower(extension);
  }
  if (directory_.empty()) {
    directory_ = DefaultDirectory();
  }
  selected_.clear();
  open_ = true;
  Refresh();
}

void FileBrowser::Close() {
  open_ = false;
  selected_.clear();
}

bool FileBrowser::Matches(const std::filesystem::path& path) const {
  if (extensions_.empty()) {
    return true;
  }
  std::string extension = ToLower(path.extension().string());
  return std::find(extensions_.begin(), extensions_.end(), extension) != extensions_.end();
}

void FileBrowser::Navigate(const std::filesystem::path& directory) {
  std::error_code ec;
  if (!std::filesystem::is_directory(directory, ec)) {
    return;
  }
  directory_ = std::filesystem::absolute(directory, ec).lexically_normal();
  selected_.clear();
  Refresh();
}

void FileBrowser::Refresh() {
  entries_.clear();
  error_.clear();

  std::error_code ec;
  std::filesystem::directory_iterator it{directory_, ec};
  if (ec) {
    error_ = ec.message();
    return;
  }
  for (const std::filesystem::directory_entry& entry :
       std::filesystem::directory_iterator{directory_, ec}) {
    std::error_code entry_ec;
    bool is_directory = entry.is_directory(entry_ec);
    if (entry_ec) {
      continue;
    }
    std::string name = entry.path().filename().string();
    if (!name.empty() && name[0] == '.') {
      continue;
    }
    if (!is_directory && !Matches(entry.path())) {
      continue;
    }
    entries_.push_back({name, is_directory});
  }

  // Directories first, then files, each alphabetically.
  std::sort(entries_.begin(), entries_.end(), [](const Entry& a, const Entry& b) {
    if (a.is_directory != b.is_directory) {
      return a.is_directory;
    }
    return ToLower(a.name) < ToLower(b.name);
  });
}

bool FileBrowser::Draw(std::string& selection) {
  if (!open_) {
    return false;
  }

  ImGui::SetNextWindowSize(ImVec2(520, 380), ImGuiCond_FirstUseEver);
  bool still_open = true;
  if (!ImGui::Begin(title_.c_str(), &still_open)) {
    ImGui::End();
    if (!still_open) {
      Close();
    }
    return false;
  }

  ImGui::TextUnformatted(directory_.string().c_str());
  ImGui::Separator();

  bool chosen = false;
  float footer = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;
  ImGui::BeginChild("entries", ImVec2(0, -footer), true);

  if (directory_.has_parent_path() && directory_.parent_path() != directory_) {
    if (ImGui::Selectable("..", false, ImGuiSelectableFlags_AllowDoubleClick) &&
        ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
      Navigate(directory_.parent_path());
    }
  }

  if (!error_.empty()) {
    ImGui::TextUnformatted(("cannot read directory: " + error_).c_str());
  }

  std::filesystem::path navigate_to;
  for (const Entry& entry : entries_) {
    std::string label = entry.is_directory ? "[" + entry.name + "]" : entry.name;
    bool is_selected = !entry.is_directory && entry.name == selected_;
    if (ImGui::Selectable(label.c_str(), is_selected, ImGuiSelectableFlags_AllowDoubleClick)) {
      if (entry.is_directory) {
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
          navigate_to = directory_ / entry.name;
        }
      } else {
        selected_ = entry.name;
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
          chosen = true;
        }
      }
    }
  }
  ImGui::EndChild();

  ImGui::BeginDisabled(selected_.empty());
  if (ImGui::Button("Open")) {
    chosen = true;
  }
  ImGui::EndDisabled();
  ImGui::SameLine();
  if (ImGui::Button("Cancel")) {
    still_open = false;
  }
  if (!selected_.empty()) {
    ImGui::SameLine();
    ImGui::TextUnformatted(selected_.c_str());
  }

  ImGui::End();

  if (!navigate_to.empty()) {
    Navigate(navigate_to);
    return false;
  }
  if (chosen && !selected_.empty()) {
    selection = (directory_ / selected_).string();
    Close();
    return true;
  }
  if (!still_open) {
    Close();
  }
  return false;
}
