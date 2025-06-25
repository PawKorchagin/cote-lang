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
    vm.code[vm.code_size++] = opcode(OP_ADD, 0, 0, 1);
    vm.code[vm.code_size++] = opcode(OP_RETURN, 0, 0, 0);
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

TEST(SimpleJitTest, Playground) {
    using namespace asmjit;
    JitRuntime asmrt;
    CodeHolder holder;
    using ftype = void *(*)(int *);
    ftype res;
    FileLogger logger(stdout);
    holder.setLogger(&logger);
    holder.init(asmrt.environment(), asmrt.cpuFeatures());
    x86::Compiler cc(&holder);

    //                                                          Value* (Value* v)
    FuncNode *node = cc.addFunc(FuncSignature::build<void *, int *>());

    std::vector<Value> mstack;
    Value val;
    val.set_int(9);
    val.set_int(10);
    mstack.push_back(val);


    x86::Gp arg1 = cc.newIntPtr("args*");       // Create `dst` register (destination pointer).
    node->setArg(0, arg1);

    int *x = new int(10);

    cc.mov(x86::word_ptr(arg1), 12);
    cc.ret(arg1);

    cc.endFunc();
    cc.finalize();
    asmjit::Error err = asmrt.add(&res, &holder);          // Add the generated code to the runtime.
    ASSERT_FALSE(err);
    auto it = res(static_cast<int *>(x));
    ASSERT_EQ(*((int *) it), 12);
//    ASSERT_FALSE(asmrt.allocator()->release(it));
    std::cout << "HERE\n";
}
