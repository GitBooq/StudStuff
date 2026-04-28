cmake_minimum_required(VERSION 3.22...4.1)

project(HWMod3 LANGUAGES CXX)

set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

option(BUILD_TASK1 "1-containers-common" ON)
option(BUILD_TASK2 "2-algorithms-common" ON)
option(BUILD_TASK3 "3-red-black-tree-impl" ON)
option(BUILD_TASK4 "4-hash-table-chaining" ON)
option(BUILD_TASK5 "5-perf" ON)

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

# GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
    GIT_SHALLOW TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

if(BUILD_TASK1)
    add_subdirectory(1-containers-common)
endif()

if(BUILD_TASK2)
    add_subdirectory(2-algorithms-common)
endif()

if(BUILD_TASK3)
    add_subdirectory(3-red-black-tree-impl)
endif()

if(BUILD_TASK4)
    add_subdirectory(4-hash-table-chaining)
endif()

if(BUILD_TASK5)
    #GoogleBenchmark
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    
    FetchContent_Declare(
        googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.9.5
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(googlebenchmark)

    # Compile benchmark library with optimisations
    target_compile_options(benchmark PRIVATE -O2 -DNDEBUG)
    target_compile_options(benchmark_main PRIVATE -O2 -DNDEBUG)

    add_subdirectory(5-perf)
endif()