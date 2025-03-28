cmake_minimum_required(VERSION 3.12)
project(crypt VERSION 1.0 DESCRIPTION "Crypt Compiler" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lib)
add_subdirectory(bin)

enable_testing()
add_subdirectory(tests)
