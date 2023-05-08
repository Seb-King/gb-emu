#include <gtest/gtest.h>
#include <string>
#include "../cartridge.hpp"

TEST(Cart_Factory_Tests, ASSERTIONS) {
  std::vector<u8> memory(0x8000, 0);
  memory.at(0x0147) = 0x00;
  Cartridge* cart = cart_factory(memory);

  EXPECT_TRUE(cart->cart_type() == "NoMBC");
}

TEST(Cart_Tests, READ_ASSERTIONS) {
  std::vector<u8> memory(0x8000, 1);
  memory.at(0x0147) = 0x00;
  Cartridge* cart = cart_factory(memory);

  EXPECT_EQ(cart->read(0x0001), 0x0001);
}
