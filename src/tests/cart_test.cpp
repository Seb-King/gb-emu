#include <gtest/gtest.h>
#include <string>
#include "../cartridge.hpp"

TEST(Cart_Factory_Tests, NoMBC) {
  std::vector<u8> memory(0x8000, 0);
  memory.at(0x0147) = 0x00;
  Cartridge* cart = cart_factory(memory);

  EXPECT_TRUE(cart->cart_type() == "NoMBC");
}

TEST(Cart_Factory_Tests, MBC3) {
  std::vector<u8> memory(0x8000, 0);
  memory.at(0x0147) = 0x0F;
  Cartridge* cart = cart_factory(memory);

  EXPECT_TRUE(cart->cart_type() == "MBC3");
}

TEST(NoMBC_Cart_Tests, READ_AND_WRITES_ASSERTIONS) {
  std::vector<u8> memory(0xC000, 1);
  memory.at(0x0147) = 0x00;
  Cartridge* cart = cart_factory(memory);

  EXPECT_EQ(cart->read(0x0001), 0x01);

  cart->write(0x12, 0x0333);
  EXPECT_EQ(cart->read(0x0333), 0x01);

  cart->write(0x10, 0xA000);
  EXPECT_EQ(cart->read(0xA000), 0x10);
}
