add_executable(tests_all
    grpc_message_sender_test.cc
)

target_link_libraries(tests_all
    sender_lib
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
)

target_compile_options(tests_all PRIVATE
    -O0
    -g
)

include(GoogleTest)
gtest_discover_tests(tests_all)
