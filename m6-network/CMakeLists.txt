cmake_minimum_required(VERSION 3.20)

project(DataTransfer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
option(ENABLE_SANITIZERS "Enable sanitizers" OFF)

option(ENABLE_TESTS "Enable tests" OFF)

if (MSVC)
    # warning level 4
    add_compile_options(/W4)
else()
    # additional warnings
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if (ENABLE_SANITIZERS)
    add_compile_options(-fsanitize=address,undefined -g -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Protobuf_VERSION = ${Protobuf_VERSION}")
find_package(gRPC CONFIG REQUIRED)

add_subdirectory(proto)

set(NETLOGFILTER_BUILD_TESTS OFF)
set(NETLOGFILTER_BUILD_EXAMPLES OFF)

include(FetchContent)
FetchContent_Declare(
    netLogFilter
    GIT_REPOSITORY https://github.com/GitBooq/net-log-filter.git
    GIT_TAG v1.0.2
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(netLogFilter)

include_directories(${PROJECT_SOURCE_DIR}/include)
add_subdirectory(src/sender)
add_subdirectory(src/receiver)
add_subdirectory(src/common)

if (ENABLE_TESTS)
  # GoogleTest
  include(FetchContent)
  FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG v1.17.0
      GIT_SHALLOW TRUE
      GIT_PROGRESS TRUE
  )
  # For Windows: Prevent overriding the parent project's compiler/linker settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
  enable_testing()
  add_subdirectory(tests)
endif()