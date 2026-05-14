cmake_minimum_required(VERSION 3.22...4.1)

project(HWMod5 LANGUAGES CXX)

set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
option(ENABLE_SANITIZERS "Enable sanitizers" OFF)
option(ENABLE_TSAN "Enable TSan" ON)

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

if (ENABLE_TSAN)
    add_compile_options(-fsanitize=thread -fPIE -pie -g -O2)
    add_link_options(-fsanitize=thread)
endif()

add_library(ThreadPool thread_pool.cc)
target_compile_features(ThreadPool PRIVATE cxx_std_23)

add_executable(Main main.cc)
target_link_libraries(Main PRIVATE ThreadPool)
target_compile_features(Main PRIVATE cxx_std_23)