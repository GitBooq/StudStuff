cmake_minimum_required(VERSION 3.20)

project(cppapp)

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

find_package(libpqxx CONFIG REQUIRED)

# Core library (Entities + Use Cases)
add_library(core INTERFACE)
target_include_directories(core INTERFACE ${CMAKE_SOURCE_DIR}/src)

# Adapters library (Repository interfaces)
add_library(adapters INTERFACE)
target_include_directories(adapters INTERFACE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(adapters INTERFACE
    core
)

add_library(infrastructure STATIC
    src/infrastructure/db/PostgresUserRepo.cc
    src/infrastructure/db/PostgresPostRepo.cc
    src/infrastructure/db/PostgresTagRepo.cc
    src/infrastructure/db/PostgresCommentRepo.cc
    src/infrastructure/db/PostgresUnitOfWork.cc
)
target_include_directories(infrastructure PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(infrastructure PUBLIC
    adapters
    libpqxx::pqxx
)

add_library(use_cases STATIC
  src/core/use_cases/CreateUserWithPost.cc
  src/core/use_cases/GetUserTagsFromComments.cc
)
target_include_directories(use_cases PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(use_cases PUBLIC
    adapters
)

add_executable(cpp_app src/main.cc)
target_link_libraries(cpp_app PRIVATE libpqxx::pqxx infrastructure use_cases)