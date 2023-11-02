#include <gtest/gtest.h>
#include "../cpu.hpp"

TEST(CPU_Tests, READ_INCREMENTS_PC) {
  GB_CPU cpu;
  cpu.PC = 0x0000;
  cpu.read();
  EXPECT_EQ(cpu.PC, 0x0001);

  cpu.read();
  EXPECT_EQ(cpu.PC, 0x0002);
}
