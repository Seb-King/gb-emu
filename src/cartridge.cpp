#include "typedefs.hpp"
#include "cartridge.hpp"

void Cartridge::init(std::vector<u8> mem) {
  memory = mem;
}

u8 Cartridge::read(u16 addr) {
  return memory.at(addr);
}

void Cartridge::write(u8 val, u16 addr) {}

class NoMemoryBankControllerCartridge : public Cartridge {
  vector<u8> memory;
public:
  void write(u8 val, u16 addr) {
    if (addr >= 0xA000 && addr <= 0xBFFF)
      memory.at(addr) = val;
  }
};

Cartridge* cart_factory(std::vector<u8> game_data) {
  Cartridge* cart = new NoMemoryBankControllerCartridge();
  cart->init(game_data);
  return cart;
}
