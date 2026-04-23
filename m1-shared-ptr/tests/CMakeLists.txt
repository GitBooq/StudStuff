add_executable(SharedPtrTest
    smart_shared_ptr_test.cc
)

target_link_libraries(SharedPtrTest
    SharedPtrLib
    GTest::gtest_main
)

target_compile_options(SharedPtrTest PRIVATE
    -O0
    -g
)

include(GoogleTest)
gtest_discover_tests(SharedPtrTest)

