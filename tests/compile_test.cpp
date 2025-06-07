//
// Created by motya on 06.06.2025.
//
#include "utils.h"
#include "lib/ast.h"

using SimpleCompileFromFileOk = Test;

inline void compile_program(std::istream& fin, const std::string& file_name = "code") {
    parser::init_parser(fin, new BytecodeEmitter());
    ast::Program p = parser::parse_program();
    print_func_body(p.instructions);

    if (!parser::get_errors().empty()) {
        for (auto x : get_errors()) {
            std::cerr << file_name << ":" << x << std::endl;
        }
        throw std::runtime_error("parser failed: " + get_errors().front());
    }
}

TEST(SimpleCompileFromFileOk, FileTests) {
    EXPECT_NO_THROW({
        std::ifstream fin("../../tests/sources/test2.ct");
        return compile_program(fin, "../../tests/sources/test2.ct");
    });
}