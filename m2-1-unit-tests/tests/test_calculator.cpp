#include "Calculator.h"
#include <gtest/gtest.h>
#include <stdexcept>

// Task 1: Basic tests
TEST(CalculatorTest, AddPositiveNumbers) {
  Calculator calc;
  EXPECT_EQ(calc.Add(2, 3), 5);
}

TEST(CalculatorTest, AddNegativeNumbers) {
  Calculator calc;
  EXPECT_EQ(calc.Add(-2, -3), -5);
}

TEST(CalculatorTest, SubtractPositiveNumbers) {
  Calculator calc;
  EXPECT_EQ(calc.Subtract(5, 3), 2);
}

TEST(CalculatorTest, MultiplyPositiveNumbers) {
  Calculator calc;
  EXPECT_EQ(calc.Multiply(2, 3), 6);
}

TEST(CalculatorTest, IsEvenWithEvenNumber) {
  Calculator calc;
  EXPECT_TRUE(calc.IsEven(4));
}

TEST(CalculatorTest, IsEvenWithOddNumber) {
  Calculator calc;
  EXPECT_FALSE(calc.IsEven(5));
}

// Task 2: Exceptions tests
TEST(CalculatorTest, DivideByZeroThrowsException) {
  Calculator calc;
  ASSERT_THROW(calc.Divide(10, 0), std::invalid_argument);
}

// Task 3: Fixtures
class CalculatorTestF : public ::testing::Test {
protected:
  void SetUp() override {}
  Calculator calc;
};

TEST_F(CalculatorTestF, AddWithFixture) { EXPECT_EQ(calc.Add(2, 3), 5); }

TEST_F(CalculatorTestF, SubtractWithFixture) {
  EXPECT_EQ(calc.Subtract(5, 3), 2);
}