//
// Created by Георгий on 21.06.2025.
//

#include "utils.h"
#include "src/ast.h"
#include "src/ins_to_string.h"

using namespace interpreter;
using SimpleCompileFromFileOk = Test;

inline void compile_program(std::istream &fin, const std::string &file_name = "code") {
    using namespace interpreter;
    auto &vm = initVM();
    parser::init_parser(fin, new BytecodeEmitter());
    ASSERT_NO_THROW(parser::parse_program(vm));
    print_vm_data(vm);
    vm.jit_log_level = 1;
    interpreter::run(true);
    ASSERT_TRUE(vm.call_stack.empty());
    ASSERT_EQ(vm.stack[0].i32, 0);

    if (!parser::get_errors().empty()) {
        for (auto x: get_errors()) {
            std::cerr << file_name << ":" << x <<
                      std::endl;
        }
        throw std::runtime_error("parser failed: " + get_errors().front());
    }
}

// using SimpleCompileFromFileParam = TestWithParam<std::string>;

TEST(SimpleCompileFromFileOk, TestFloatArithmetic) {
    ASSERT_NO_THROW({
                        std::ifstream fin("../../tests/sources/floatarithmetic.ct");
                        return compile_program(fin);
                    });
}

TEST(SimpleCompileFromFileOk, TestJit1) {
    ASSERT_NO_THROW({
                        std::ifstream fin("../../tests/sources/jitSimple.ct");
                        return compile_program(fin);
                    });
}

TEST(SimpleCompileFromFileOk, Test3) {
    ASSERT_NO_THROW({
                        std::ifstream fin("../../tests/sources/test3.ct");
                        return compile_program(fin);
                    });
}


TEST(SimpleCompileFromFileOk, TestJitTemp) {
    ASSERT_NO_THROW({
                        std::ifstream fin("../../tests/sources/jitSimple2.ct");
                        return compile_program(fin);
                    });
}


TEST(SimpleCompileFromFileOk, Test4) {
    ASSERT_NO_THROW({
                        std::ifstream fin("../../tests/sources/test4.ct");
                        return compile_program(fin);
                    });
}

TEST(SimpleCompileFromFileOk, Test5) {
    ASSERT_NO_THROW({
                        std::ifstream fin("../../tests/sources/test5.ct");
                        return compile_program(fin);
                    });
}

TEST(SimpleCompileFromFileOk, Test6) {
    ASSERT_NO_THROW({
                        std::ifstream fin("../../tests/sources/test6.ct");
                        return compile_program(fin);
                    });
}

TEST(SimpleCompileFromFileOk, Test6mini) {
    ASSERT_NO_THROW({
                        std::ifstream fin("../../tests/sources/test6mini.ct");
                        return compile_program(fin);
                    });
}

TEST(SimpleCompileFromFileOk, Test7) {
    ASSERT_NO_THROW({
                        std::ifstream fin("../../tests/sources/test7.ct");
                        return compile_program(fin);
                    });
}


TEST(SimpleCompileFromFileOk, Test8) {
    ASSERT_NO_THROW({
                        std::ifstream fin("../../tests/sources/test8.ct");
                        return compile_program(fin);
                    });
}

TEST(SimpleCompileFromFileOk, TestGC1) {
    ASSERT_NO_THROW({
                        std::ifstream fin("../../tests/sources/gc/test_arraylink_mini.ct");
                        return compile_program(fin);
                    });
}

TEST(SimpleCompileFromFileOk, TestInner) {
    ASSERT_NO_THROW({
                        std::ifstream fin("../../tests/sources/gc/test_innerlink.ct");
                        return compile_program(fin);
                    });
}

TEST(SimpleCompileFromFileOk, TestInnerMini) {
    ASSERT_NO_THROW({
                        std::ifstream fin("../../tests/sources/gc/test_innerlink_mini.ct");
                        return compile_program(fin);
                    });
}

// INSTANTIATE_TEST_SUITE_P(
//     CorrectGroup,
//     SimpleCompileFromFileParam,
//     Values(
//         "test3.ct",
//         "test4.ct",
//         "test5.ct"
//     )
// );

// int main(int argc, char** argv) {
//     InitGoogleTest(&argc, argv);
//     //std::cout << "Registered tests:\n";
//     // auto& listeners = UnitTest::GetInstance()->listeners();
//     // listeners.Append(new FileAndConsoleListener("logs/dump"));
//     return RUN_ALL_TESTS();
// }

//using SimpleLayeredTest = TestWithParam<std::vector<void*(int)>>;
//TEST_P(SimpleLayeredTest, FileTest1) {
//
//}
