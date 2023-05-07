#include <gtest/gtest.h>
#include "../cartridge.hpp"

TEST(Cart_Tests, READ_ASSERTIONS) {
  std::vector<u8> memory(0x8000, 1);
  Cartridge cart;
  cart.init(memory);

  EXPECT_EQ(cart.read(0x0001), 0x0001);
}