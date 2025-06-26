////
//// Created by motya on 06.06.2025.
////
#include "utils.h"
#include "src/ast.h"
#include "src/jit_runtime.h"
#include "libs/asmjit/src/asmjit/x86.h"

using namespace interpreter;
using SimpleJitTest = Test;

TEST(SimpleJitTest, Test1) {
    VMData &vm = vm_instance();
    initVM();
    Value v;
    v.set_int(2);
    vm.constanti.push_back(v);
    v.set_int(3);
    vm.constanti.push_back(v);
    vm.code[vm.code_size++] = opcode(OP_ADD, 2, 0, 1);
    vm.code[vm.code_size++] = opcode(OP_RETURN, 2, 0, 0);
    vm.functions[0].code_size = vm.code_size;
    asmjit::JitRuntime jt;
    jit::JitRuntime rt;
    jit::FuncCompiled func;
    vm.functions[0].arity = 2;
    rt.compile(vm, vm.functions[0], func, vm.functions[0].arity);


    Value res;
    res.set_int(5);
    auto temp = *reinterpret_cast<uint64_t *>(&res);
    vm.stack[0].set_int(2.0f);
    vm.stack[1].set_int(3.0f);
//    return;
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);

    std::cout << func(vm.stack, &vm.functions[0]) << std::endl;

    res.set_float(5.0);
    temp = *reinterpret_cast<uint64_t *>(&res);
    vm.stack[0].set_float(2.0f);
    vm.stack[1].set_float(3.0f);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);

    std::cout << func(vm.stack, &vm.functions[0]) << std::endl;

    res.set_nil();
    res.i32 += 1;
    temp = *reinterpret_cast<uint64_t *>(&res);
    vm.stack[0].set_float(2.0f);
    vm.stack[1].set_int(3.0f);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);

}

void my_cpp_function(int x, int y) {
    std::cout << "Called from assembler with x = " << x << ' ' << y << std::endl;

}

asmjit::x86::Gp getArg1() {
#if defined(_WIN32)
    return asmjit::x86::rcx;
#else
    return asmjit::x86::rdi;
#endif
}

asmjit::x86::Gp getArg2() {
#if defined(_WIN32)
    return asmjit::x86::rdx;
#else
    return asmjit::x86::rsi;
#endif
}

TEST(SimpleJitTest, Playground) {
    using namespace asmjit;

    JitRuntime rt;
    CodeHolder code;
    code.init(rt.environment());

    x86::Assembler a(&code);

    // Optional: Align stack to 16 bytes if needed before calling
    // According to ABI, stack must be aligned to 16 bytes *at the point of call*

    // Push rbp to preserve base pointer (for debugging etc.)
    a.push(x86::rbp);
    a.mov(x86::rbp, x86::rsp);

    // Align stack if needed
    // Call instruction pushes return address (8 bytes), so we subtract 8 to align to 16
    a.sub(x86::rsp, 16);  // 16-byte alignment


    a.mov(getArg1(), x86::rsp);  // Argument
    a.mov(getArg2(), 10);  // Argument
    a.mov(x86::rax, imm((uintptr_t) (void *) my_cpp_function));
    a.call(x86::rax);
    a.leave();
    a.ret();

    // Compile and run
    using Func = void (*)();
    Func fn;
    if (rt.add(&fn, &code) != kErrorOk) {
        std::cerr << "Failed to compile function" << std::endl;
        return;
    }

    fn();

    rt.release(fn);
}
