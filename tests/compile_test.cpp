//
// Created by motya on 06.06.2025.
//
#include "utils.h"
#include "lib/ast.h"

using namespace interpreter;
using SimpleCompileFromFileOk = Test;

inline void compile_program(std::istream& fin, const std::string& file_name = "code") {
    using namespace interpreter;
    parser::init_parser(fin, new BytecodeEmitter());
    auto& vm = initVM();
    ast::Program p = parser::parse_program(vm);
    vm.code[vm.code_size++] = opcode(OP_CALL, 0, 0, 0);
    vm.code[vm.code_size++] = opcode(OP_HALT);
    vm.functions[0] = Function();
    vm.functions[0].arity = 0;
    vm.functions[0].entry_point = 0;
    vm.functions[0].local_count = 100;
    vm.ip = vm.code_size - 2;
    std::cout << "[";
    for (int i = 0; i < vm.constants.size(); ++i) {
        std::cout << vm.constants[i].as.i32 << ", ";
    }
    std::cout << "]\n";
    print_func_body(p.instructions);
    interpreter::run();
    ASSERT_TRUE(vm.call_stack.empty());
    ASSERT_EQ(vm.stack[0].as.i32, 55);


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