add_library(crypt_lib STATIC parser.cpp ast.cpp lexer.cpp equals.h vm.cpp vm.h exceptions.cpp
        concept.h)

target_include_directories(crypt_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
