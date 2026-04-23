#include "student_list.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdexcept>
#include <string_view>

using namespace std::literals::string_literals;

[[maybe_unused]] constexpr std::string_view kGoodData{"John 1\nDoe 2\nBob 10"};
[[maybe_unused]] constexpr std::string_view kBadFormatData1{
    "John 1\nDoe two\nBob 10"};
[[maybe_unused]] constexpr std::string_view kBadAfterData{
    "John 1\nDoe 2\nBob 10a"};
[[maybe_unused]] constexpr std::string_view kBadFormatData2{
    "John 1\nDoe 2\nBob "};
[[maybe_unused]] constexpr std::string_view kDuplicateData{
    "John 1\nDoe 2\nDoe 3"};
[[maybe_unused]] constexpr std::string_view kNegativeGradeData{
    "John 1\nDoe 2\nBob -10"};

auto MakeStream(std::string_view data) {
  return std::istringstream(std::string(data));
}

TEST(StudentListTest, ReadFromFileGood) {
  auto iss = MakeStream(kGoodData);
  StudentList lst;

  EXPECT_NO_THROW(lst.Read(iss));
}

class StudentListTestF : public ::testing::Test {
protected:
  void SetUp() override {
    auto iss = MakeStream(kGoodData);
    lst_.Read(iss);
  }

  StudentList lst_;
};

TEST_F(StudentListTestF, Output) {
  std::ostringstream oss;
  EXPECT_NO_THROW(lst_.Output(oss));

  std::string_view expect = "Bob 10\nDoe 2\nJohn 1\n";
  EXPECT_EQ(oss.view(), expect);
}

TEST_F(StudentListTestF, GetMaxGradeStudent) {
  std::pair<const std::string, int> max_grade_student{"Bob"s, 10};
  auto max_grade_student_it = lst_.GetMaxGradeStudent();

  ASSERT_NE(max_grade_student_it, lst_.End());
  EXPECT_EQ(max_grade_student, *max_grade_student_it);
}

TEST_F(StudentListTestF, GetAvgGrade) {
  auto avg_grade_et = static_cast<double>(1 + 2 + 10) / 3;
  double avg_grade = lst_.GetAvgGrade();

  EXPECT_EQ(avg_grade, avg_grade_et);
}

////////////////////
// Bad input data //
////////////////////

class StudentListInvalidReadTest
    : public ::testing::TestWithParam<std::tuple<std::string, std::string>> {};

TEST_P(StudentListInvalidReadTest, ThrowsWithCorrectMessage) {
  StudentList list;
  auto [data, expected] = GetParam();
  auto iss = MakeStream(data);

  try {
    list.Read(iss);
    FAIL() << "Expected std::runtime_error"; // should not go there
  } catch (const std::runtime_error &e) {
    EXPECT_EQ(e.what(), expected);
  } catch (...) {
    FAIL() << "Expected std::runtime_error, got different exception";
  }
}

INSTANTIATE_TEST_SUITE_P(
    InvalidInputs, StudentListInvalidReadTest,
    ::testing::Values(
        std::make_tuple(kBadFormatData1, "Invalid line format"),
        std::make_tuple(kBadAfterData, "Extra data in line: Bob 10a"),
        std::make_tuple(kBadFormatData2, "Invalid line format"),
        std::make_tuple(kDuplicateData, "Duplicate student: Doe"),
        std::make_tuple(kNegativeGradeData, "Grade can't be negative")));