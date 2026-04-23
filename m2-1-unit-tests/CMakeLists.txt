cmake_minimum_required(VERSION 3.14)
project(CalculatorWithTests)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(TESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.17.0
  GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_library(Calculator ${SRC_DIR}/Calculator.cpp)
target_include_directories(Calculator PUBLIC ${INCLUDE_DIR})

add_executable(CalculatorTests ${TESTS_DIR}/test_calculator.cpp)
target_link_libraries(CalculatorTests PRIVATE gtest_main Calculator)

add_test(NAME CalculatorTests COMMAND CalculatorTests)