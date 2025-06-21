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
        cote_test
        parser_test.cpp
        vm_test.cpp
        parser_test_param.cpp
        jit_tests.cpp
        utils.h
        compile_test_param.cpp
        gc_test.cpp
)

target_link_libraries(cote_test PRIVATE crypt_lib GTest::gtest_main)

# --------------------------------------------------------------------------------
# Sanitizers: enable AddressSanitizer and LeakSanitizer, ignore for Windows
# --------------------------------------------------------------------------------
if(NOT WIN32)
    message(STATUS "Enabling AddressSanitizer and LeakSanitizer for cote test")
    target_compile_options(cote_test PRIVATE
            -fsanitize=address
#            -fsanitize=leak
            -O1                 # ?
            -fno-omit-frame-pointer
    )
    target_link_options(cote_test PRIVATE
            -fsanitize=address
#            -fsanitize=leak
    )
endif()

target_include_directories(cote_test PUBLIC ${PROJECT_SOURCE_DIR})

include(GoogleTest)
gtest_discover_tests(cote_test)
