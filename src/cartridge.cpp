#include "typedefs.hpp"
#include "cartridge.hpp"
#include <string>

using namespace std;

class NoMemoryBankControllerCartridge : public Cartridge {
  vector<u8> memory;
public:
  string cart_type() {
    return "NoMBC";
  };

  void init(std::vector<u8> mem) {
    memory = mem;
  }

  void write(u8 val, u16 addr) {
    if (addr >= 0xA000 && addr <= 0xBFFF)
      memory.at(addr) = val;
  }

  u8 read(u16 addr) {
    return memory.at(addr);
  }
};

Cartridge* cart_factory(std::vector<u8> game_data) {
  Cartridge* cart = new NoMemoryBankControllerCartridge();
  cart->init(game_data);
  return cart;
}
