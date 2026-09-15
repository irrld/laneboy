#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "util.h"

// An ImGui file picker. The platform dialogs tinyfiledialogs shells out to are
// not dependable on Linux: it decides a session is headless when DISPLAY is
// unset, which is the case under Wayland, and with no zenity, kdialog or yad
// installed it falls back to reading stdin, which hangs the render thread.
// Drawing the picker ourselves keeps it inside the frame and behaves the same
// on every platform.
class FileBrowser {
 public:
  // Extensions are matched case insensitively and include the dot, ".gb".
  void Open(std::string title, std::vector<std::string> extensions);
  void Close();

  // Draws the dialog when open. Returns true on the frame a file is chosen,
  // with the absolute path written to selection.
  bool Draw(std::string& selection);

  bool is_open() const { return open_; }

 private:
  struct Entry {
    std::string name;
    bool is_directory;
  };

  void Navigate(const std::filesystem::path& directory);
  void Refresh();
  bool Matches(const std::filesystem::path& path) const;

  bool open_ = false;
  std::string title_;
  std::vector<std::string> extensions_;
  std::filesystem::path directory_;
  std::string selected_;
  std::vector<Entry> entries_;
  std::string error_;
};
