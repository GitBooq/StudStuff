add_library(RBTree INTERFACE red_black_tree.h)
target_compile_features(RBTree INTERFACE cxx_std_23)

add_executable(RBTreeTest red_black_tree_test.cc)
target_link_libraries(RBTreeTest 
    RBTree
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(RBTreeTest)