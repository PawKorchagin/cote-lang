cmake_minimum_required(VERSION 3.12)

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
        parser_test
        parser_test.cpp
        vm_test.cpp
        parser_test_param.cpp
        utils.h
        analyze_param_test.cpp
)

target_link_libraries(parser_test PRIVATE crypt_lib GTest::gtest_main)

target_include_directories(parser_test PUBLIC ${PROJECT_SOURCE_DIR})

include(GoogleTest)
gtest_discover_tests(parser_test)
