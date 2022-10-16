find_package(GTest REQUIRED)
enable_testing()

file(GLOB_RECURSE GB_TEST "*.cpp" "*.h")

add_executable(
  megb-test
  ${GB_TEST}
)

target_include_directories(megb-test PUBLIC ../gb-lib)

target_link_libraries(
  megb-test
  GTest::gtest_main
  megb-lib
)

include(GoogleTest)
gtest_discover_tests(megb-test)