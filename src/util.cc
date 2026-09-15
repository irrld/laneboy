#include "util.h"
#include <fstream>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

namespace {

std::filesystem::path ExecutableDir() {
#if defined(_WIN32)
  wchar_t buffer[MAX_PATH];
  DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
  if (length == 0 || length == MAX_PATH) {
    return {};
  }
  return std::filesystem::path(buffer, buffer + length).parent_path();
#elif defined(__APPLE__)
  u32 size = 0;
  _NSGetExecutablePath(nullptr, &size);
  std::string buffer(size, '\0');
  if (_NSGetExecutablePath(buffer.data(), &size) != 0) {
    return {};
  }
  return std::filesystem::path(buffer.c_str()).parent_path();
#else
  std::error_code ec;
  std::filesystem::path self = std::filesystem::read_symlink("/proc/self/exe", ec);
  if (ec) {
    return {};
  }
  return self.parent_path();
#endif
}

}  // namespace

std::vector<u8> LoadBin(const std::string& path) {
  std::ifstream input(path, std::ios::binary);
  if (!input.is_open()) {
    // handle error
    throw std::runtime_error("Unable to open file: " + path);
  }

  // get the size of the file
  input.seekg(0, std::ios::end);
  std::streamsize size = input.tellg();
  input.seekg(0, std::ios::beg);

  std::vector<u8> buffer(size);
  if (input.read((char*) buffer.data(), size)) {
    return buffer;
  } else {
    // handle error
    throw std::runtime_error("Error reading file: " + path);
  }
}

std::filesystem::path AssetPath(const std::string& relative) {
  std::error_code ec;
  std::filesystem::path exe_dir = ExecutableDir();
  if (!exe_dir.empty()) {
    std::filesystem::path candidate = exe_dir / relative;
    if (std::filesystem::exists(candidate, ec)) {
      return candidate;
    }
  }
  std::filesystem::path candidate = std::filesystem::current_path(ec) / relative;
  if (!ec && std::filesystem::exists(candidate, ec)) {
    return candidate;
  }
  return {};
}
