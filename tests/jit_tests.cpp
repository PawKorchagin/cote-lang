//
// Created by motya on 06.06.2025.
//
#include "utils.h"
#include "src/ast.h"
#include "src/nodes.h"
#include "libs/asmjit/src/asmjit/core.h"
#include "libs/asmjit/src/asmjit/x86.h"

using namespace interpreter;
using SimpleJitTest = Test;

inline void compile_program_(std::istream &fin, const std::string &file_name = "code") {
    using namespace interpreter;
    auto &vm = initVM();
    parser::init_parser(fin, new BytecodeEmitter());
    ast::Program p;
    ASSERT_NO_THROW(p = parser::parse_program(vm));
    print_vm_data(vm);
    cfg::VMInfo vmInfo(vm);

    vmInfo.code = vm.code;
    vmInfo.code_size = vm.code_size;
    cfg::CFGraph graph(vmInfo);
    ASSERT_TRUE(graph.buildBasicCFG());
    graph.toString(std::cout);
    interpreter::run();
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

typedef int (*Func1)(Value* stack);

asmjit::x86::Gp get_arg1(asmjit::x86::Assembler& a) {
    using namespace asmjit;
    x86::Gp res;
    if (ASMJIT_ARCH_BITS == 64) {
#if defined(_WIN32)
        res = x86::rcx;                 // First argument (array ptr).
#else
        res = x86::rdi;                 // First argument (array ptr).
#endif
    }
    else {
        res = x86::edx;                 // Use EDX to hold the array pointer.
        // Fetch first and second arguments from [ESP + 4] and [ESP + 8].
        a.mov(res, x86::ptr(x86::esp, 4));
    }
    return res;
}

TEST(SimpleJitTest, Test1) {
    using namespace asmjit;
    asmjit::JitRuntime rt;
    CodeHolder holder;
    holder.init(rt.environment(), rt.cpuFeatures());
    {
        asmjit::x86::Assembler a(&holder);
        x86::Gp stack = get_arg1(a);
        a.mov(x86::eax, x86::dword_ptr(stack, 4));
        a.ret();
    }
    Func1 func;
    Error err = rt.add(&func, &holder);

    ASSERT_EQ(err, 0);
    Value v = Value{};
    v.set_int(100);
    int64_t result = func(&v);

    ASSERT_EQ(result, 100);
    rt.release(func);

    std::ifstream fin("../../tests/sources/test3.ct");
    compile_program_(fin, "ff");
}