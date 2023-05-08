#include <vector>
#include "typedefs.hpp"

using namespace std;

class Cartridge {
  vector<u8> memory;
public:
  virtual string cart_type() = 0;
  virtual void init(std::vector<u8> memory) = 0;
  virtual u8 read(u16 addr) = 0;
  virtual void write(u8 val, u16 addr) = 0;
};

Cartridge* cart_factory(std::vector<u8> game_data);
