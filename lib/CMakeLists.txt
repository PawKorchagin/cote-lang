add_library(crypt_lib STATIC parser.cpp AST.h equals.h parser.h)

target_include_directories(crypt_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
