add_library(crypt_lib STATIC
        parser.cpp
        ast.cpp
        lexer.cpp
        equals.h
        vm.cpp vm.h
        exceptions.cpp
        concept.h
        nodes.cpp
        bytecode_emitter.cpp
        var_manager.cpp
        expr_semantic.cpp
        codegen.cpp
        lang_stdlib.cpp
        trace.cpp
        gc.cpp
        gc.h
        misc.cpp
)

target_link_libraries(crypt_lib asmjit::asmjit)
target_include_directories(crypt_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
