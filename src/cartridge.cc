#include "cartridge.h"


Cartridge::Cartridge(const std::string& path) {
  data_ = LoadBin(path);
  if (data_.empty()) {
    is_valid_ = false;
    return;
  }
  is_valid_ = true;
}

void Cartridge::InitBus(MemoryBus& bus) {

}