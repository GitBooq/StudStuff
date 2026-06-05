cmake_minimum_required(VERSION 3.20)

project(ipv4_grpc)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
option(ENABLE_SANITIZERS "Enable sanitizers" OFF)

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

include(FetchContent)
FetchContent_Declare(
    netLogFilter
    GIT_REPOSITORY https://github.com/GitBooq/net-log-filter.git
    GIT_TAG v1.0.1
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(netLogFilter)

add_subdirectory(services/sender)
add_subdirectory(services/receiver)