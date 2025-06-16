//
// Created by motya on 06.06.2025.
//
#include "utils.h"
#include "lib/ast.h"
#include "lib/ins_to_string.h"
#include "lib/nodes.h"

using namespace interpreter;
using SimpleCompileFromFileOk = Test;

inline void compile_program(std::istream &fin, const std::string &file_name = "code") {
    using namespace interpreter;
    parser::init_parser(fin, new BytecodeEmitter());
    auto &vm = initVM();
    ast::Program p;
    ASSERT_NO_THROW(p = parser::parse_program(vm));
    print_vm_data(vm);
//    print_func_body(p.instructions);
//    interpreter::run();
//    ASSERT_TRUE(vm.call_stack.empty());
//    ASSERT_EQ(vm.stack[0].as.i32, 0);

    if (!parser::get_errors().empty()) {
        for (auto x: get_errors()) {
            std::cerr << file_name << ":" << x <<
                      std::endl;
        }
        throw std::runtime_error("parser failed: " + get_errors().front());
    }
}

TEST(SimpleCompileFromFileOk, FileTests) {
    ASSERT_NO_THROW({
                        std::ifstream fin("../../tests/sources/test4.ct");
                        return compile_program(fin);
                    });
//    ASSERT_NO_THROW({
//                        std::ifstream fin("../../tests/sources/test3.ct");
//                        return compile_program(fin);
//                    });
//    ASSERT_NO_THROW({
//                        std::ifstream fin("../../tests/sources/test2.ct");
//                        return compile_program(fin);
//                    });
}



//using SimpleLayeredTest = TestWithParam<std::vector<void*(int)>>;
//TEST_P(SimpleLayeredTest, FileTest1) {
//
//}