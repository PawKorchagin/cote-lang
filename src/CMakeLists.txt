add_library(cote_lib STATIC
        parser.cpp
        ast.cpp
        lexer.cpp
        equals.h
        vm.cpp vm.h
        exceptions.cpp
        concept.h
        bytecode_emitter.cpp
        var_manager.cpp
        expr_semantic.cpp
        codegen.cpp
        lang_stdlib.cpp
        gc.cpp
        gc.h
        misc.cpp
        heap.cpp
        heap.h
        JITRuntime.cpp
)

target_link_libraries(cote_lib asmjit::asmjit)
target_include_directories(cote_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
