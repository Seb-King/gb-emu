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
    if (addr >= 0xA000 && addr <= 0xBFFF) {
      RAM.at(addr - 0xA000) = val;
    }
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

class MBC3Cartridge : public Cartridge {
  vector<u8> ROM;
  vector<u8> RAM;

  u8 rtc_register = 0x00;

  int rom_bank = 0;
  int ram_bank = 0;

  u8 read_rom(u16 addr) {
    if (addr >= 0 && addr <= 0x3FFF) {
      return ROM.at(addr);
    } else {
      return ROM.at(addr - 0x4000 + 0x4000 * rom_bank);
    }
  }

  u8 read_ram(u16 addr) {
    if (ram_bank >= 0x08 && ram_bank <= 0x0C) {
      return rtc_register;
    }

    return RAM.at((addr - 0xA000) + 0x2000 * ram_bank);
  }

  void set_rom_bank(u8 val) {
    if (val == 0x00) {
      rom_bank = 0x01;
    } else {
      rom_bank = 0x7F & val;
    }
  }

  void set_ram_bank(u8 val) {
    ram_bank = val;
  }

public:
  string cart_type() {
    return "MBC3";
  };

  void init(vector<u8> game_data) {
    ROM = game_data;
    RAM = *(new vector<u8>(0x2000 * 8, 0));
  }

  void write(u8 val, u16 addr) {
    if (addr >= 0x2000 && addr <= 0x3FFF) {
      set_rom_bank(val);
    } else if (addr >= 0x4000 && addr <= 0x5FFF) {
      set_ram_bank(val);
    } else if (addr >= 0xA000 && addr <= 0xBFFF) {
      if (ram_bank >= 0x08 && ram_bank <= 0x0C) {
        rtc_register = val;
      } else {
        RAM.at((addr - 0xA000) + 0x2000 * ram_bank) = val;
      }
    }
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
  Cartridge* cart;
  u8 cart_type_header = game_data.at(0x0147);
  if (cart_type_header >= 0x0F && cart_type_header <= 0x13) {
    cart = new MBC3Cartridge();
  } else {
    cart = new NoMemoryBankControllerCartridge();
  }
  cart->init(game_data);
  return cart;
}
