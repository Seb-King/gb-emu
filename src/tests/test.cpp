#include <gtest/gtest.h>
#include "../utils.hpp"

TEST(HelloTest, TestAssertions) {
  println("henlo");
  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world");
  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
}