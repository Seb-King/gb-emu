#include "typedefs.hpp"
#include "cartridge.hpp"

void Cartridge::init(std::vector<u8> mem) {
  memory = mem;
}

u8 Cartridge::read(u16 addr) {
  return memory.at(addr);
}

void Cartridge::write(u8 val, u16 addr) {}

Cartridge* cart_factory(std::vector<u8> game_data) {
  Cartridge* cart = new Cartridge();
  cart->init(game_data);
  return cart;
}
