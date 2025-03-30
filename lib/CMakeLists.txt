add_library(crypt_lib STATIC parser.cpp ast.cpp)

target_include_directories(crypt_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
