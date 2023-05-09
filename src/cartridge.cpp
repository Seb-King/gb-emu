#include "typedefs.hpp"
#include "cartridge.hpp"
#include <string>

using namespace std;

class NoMemoryBankControllerCartridge : public Cartridge {
  vector<u8> ROM;
  vector<u8> RAM;
public:
  string cart_type() {
    return "NoMBC";
  };

  void init(vector<u8> game_data) {
    ROM = game_data;
    RAM = *(new vector<u8>(0x2000, 0));
  }

  void write(u8 val, u16 addr) {
    if (addr >= 0xA000 && addr <= 0xBFFF)
      RAM.at(addr) = val;
  }

  u8 read_rom(u16 addr) {
    return ROM.at(addr);
  }

  u8 read_ram(u16 addr) {
    return RAM.at(addr - 0xA000);
  }

  u8 read(u16 addr) {
    if (addr >= 0 && addr <= 0x7FFF) {
      return read_rom(addr);
    } else if (addr >= 0xA000 && addr <= 0xBFFF) {
      return read_ram(addr);
    }

    return 0xFF;
  }
};

Cartridge* cart_factory(std::vector<u8> game_data) {
  Cartridge* cart = new NoMemoryBankControllerCartridge();
  cart->init(game_data);
  return cart;
}
