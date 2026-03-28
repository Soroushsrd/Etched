include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.2
)

FetchContent_MakeAvailable(googletest)

add_executable(etched_tests
  tokenizer_test.cpp
)

target_link_libraries(etched_tests
  PRIVATE GTest::gtest_main etched_lib
)

include(GoogleTest)
gtest_discover_tests(etched_tests)
