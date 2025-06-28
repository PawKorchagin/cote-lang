//
// Created by PawKorchagin on 21.06.2025.
//

// #define DEFAULT_GC_YOUNG_CAPACITY 21

#include "utils.h"
#include "src/ast.h"
#include "src/ins_to_string.h"

using namespace interpreter;
using SimpleCompileFromFileOk = Test;

inline void compile_program(std::istream &fin, const std::string &file_name = "code") {
    using namespace interpreter;
    // heap::mem.clear();
    auto &vm = initVM();
    vm.gc.cleanup();
#ifdef DEFAULT_GC_YOUNG_CAPACITY
    vm.gc.MAJOR_THRESHOLD = 10;
    vm.gc.LARGE_THRESHOLD = 10;
#endif
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

TEST(SimpleCompileFromFileOk, NoAlloc) {
    ASSERT_NO_THROW({
        std::ifstream fin("../../tests/sources/gc/test_no_alloc.ct" );
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

TEST(SimpleCompileFromFileOk, TestArrayLinkMini) {
    ASSERT_NO_THROW({
                        std::ifstream fin("../../tests/sources/gc/test_arraylink_mini.ct");
                        return compile_program(fin);
                    });
}

TEST(SimpleCompileFromFileOk, TestArrayLink) {
    ASSERT_NO_THROW({
        std::ifstream fin("../../tests/sources/gc/test_arraylink.ct" );
        return compile_program(fin);
        });
}

TEST(SimpleCompileFromFileOk, TestArrayLinkFromFoo) {
    ASSERT_NO_THROW({
        std::ifstream fin("../../tests/sources/gc/test_arraylink_from_foo.ct" );
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

TEST(SimpleCompileFromFileOk, TestCrazyLink) {
    ASSERT_NO_THROW({
        std::ifstream fin("../../tests/sources/gc/test_crazylink.ct" );
        return compile_program(fin);
        });
}

TEST(SimpleCompileFromFileOk, TestCycleLinkMini) {
    ASSERT_NO_THROW({
        std::ifstream fin("../../tests/sources/gc/test_cyclelink_mini.ct" );
        return compile_program(fin);
        });
}

TEST(SimpleCompileFromFileOk, TestCycleLink) {
    ASSERT_NO_THROW({
        std::ifstream fin("../../tests/sources/gc/test_cyclelink.ct" );
        return compile_program(fin);
        });
}


TEST(SimpleCompileFromFileOk, TestNulling) {
    ASSERT_NO_THROW({
        std::ifstream fin("../../tests/sources/gc/test_nulling.ct" );
        return compile_program(fin);
        });
}

TEST(SimpleCompileFromFileOk, TestPromote) {
    ASSERT_NO_THROW({
        std::ifstream fin("../../tests/sources/gc/test_promote.ct" );
        return compile_program(fin);
        });
}

TEST(SimpleCompileFromFileOk, TestGlobalCycle) {
    ASSERT_NO_THROW({
        std::ifstream fin("../../tests/sources/gc/test_global_cycle.ct" );
        return compile_program(fin);
        });
}

TEST(SimpleCompileFromFileOk, TestDeadLocalLink) {
    ASSERT_NO_THROW({
        std::ifstream fin("../../tests/sources/gc/test_dead_local_link.ct" );
        return compile_program(fin);
        });
}

TEST(SimpleCompileFromFileOk, TestSurvive) {
    ASSERT_NO_THROW({
        std::ifstream fin("../../tests/sources/gc/test_survive.ct" );
        return compile_program(fin);
        });
}

TEST(SimpleCompileFromFileOk, TestMultiCompGraph) {
    ASSERT_NO_THROW({
        std::ifstream fin("../../tests/sources/gc/test_multi_comp_graph.ct" );
        return compile_program(fin);
        });
}

