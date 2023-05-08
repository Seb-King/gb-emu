#include <vector>
#include "typedefs.hpp"

using namespace std;

class Cartridge {
  vector<u8> memory;
public:
  void init(std::vector<u8> memory);
  u8 read(u16 addr);
  void write(u8 val, u16 addr);
};

Cartridge* cart_factory(std::vector<u8> game_data);
