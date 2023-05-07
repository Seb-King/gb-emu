#include "typedefs.hpp"
#include "cartridge.hpp"

void Cartridge::init(std::vector<u8> mem) {
  memory = mem;
}

u8 Cartridge::read(u16 addr) {
  return memory.at(addr);
}