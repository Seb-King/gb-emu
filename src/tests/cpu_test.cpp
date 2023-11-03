#include <gtest/gtest.h>
#include "../cpu.hpp"

TEST(CPU_Tests, READ_INCREMENTS_PC) {
  RAM ram;
  GB_CPU cpu(ram);
  cpu.PC = 0x0000;
  cpu.read();
  EXPECT_EQ(cpu.PC, 0x0001);

  cpu.read();
  EXPECT_EQ(cpu.PC, 0x0002);
}
