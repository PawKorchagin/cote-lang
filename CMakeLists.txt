cmake_minimum_required(VERSION 3.12)
project(cote VERSION 1.0 DESCRIPTION "Crypt Compiler" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ASMJIT_STATIC TRUE)           # Force static build.
add_subdirectory(libs/asmjit)
add_subdirectory(src)
add_subdirectory(bin)

enable_testing()
add_subdirectory(tests)
