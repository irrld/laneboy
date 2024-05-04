#include "cartridge.h"


Cartridge::Cartridge(const std::string& path) {
  data_ = LoadBin(path);
  if (data_.size() <= 0) {
    is_valid_ = false;
    return;
  }

}

void Cartridge::InitBus(MemoryBus& bus) {

}