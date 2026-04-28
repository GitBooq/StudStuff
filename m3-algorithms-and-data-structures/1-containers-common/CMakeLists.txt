add_library(StudentList student_list.cc)
target_compile_features(StudentList PUBLIC cxx_std_23)

add_executable(StudentListTest student_list_tests.cc)
target_link_libraries(StudentListTest
    StudentList
    GTest::gtest_main
    GTest::gmock_main
)

include(GoogleTest)
gtest_discover_tests(StudentListTest)