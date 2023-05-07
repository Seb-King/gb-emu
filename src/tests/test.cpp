#include <gtest/gtest.h>
#include "../cpu.hpp"

TEST(CPU_Tests, READ_INCREMENTS_PC) {
  CPU::PC = 0x0000;
  CPU::read();
  EXPECT_EQ(CPU::PC, 0x0001);

  CPU::read();
  EXPECT_EQ(CPU::PC, 0x0002);
}
