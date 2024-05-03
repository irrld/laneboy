#include "util.h"
#include <fstream>

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