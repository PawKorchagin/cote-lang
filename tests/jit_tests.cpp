////
//// Created by motya on 06.06.2025.
////
#include "utils.h"
#include "src/ast.h"
#include "src/jit_runtime.h"
#include "libs/asmjit/src/asmjit/x86.h"

using namespace interpreter;
using SimpleJitTest = Test;

TEST(SimpleJitTest, TestAdd) {
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

TEST(SimpleJitTest, TestSub) {
    VMData &vm = vm_instance();
    initVM();
    vm.ip = 0;
    vm.fp = 0;
    Value v;
    v.set_int(2);
    vm.constanti.push_back(v);
    v.set_int(3);
    vm.constanti.push_back(v);
    vm.code[vm.code_size++] = opcode(OP_SUB, 2, 0, 1);
    vm.code[vm.code_size++] = opcode(OP_RETURN, 2, 0, 0);
    vm.functions[0].code_size = vm.code_size;
    asmjit::JitRuntime jt;
    jit::JitRuntime rt;
    jit::FuncCompiled func;
    vm.functions[0].arity = 2;
    rt.compile(vm, vm.functions[0], func, vm.functions[0].arity);


    Value res;
    res.set_int(-3);
    auto temp = *reinterpret_cast<uint64_t *>(&res);
    vm.stack[0].set_int(2.0f);
    vm.stack[1].set_int(5.0f);
//    return;
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);

    std::cout << func(vm.stack, &vm.functions[0]) << std::endl;

    res.set_float(-3.0);
    temp = *reinterpret_cast<uint64_t *>(&res);
    vm.stack[0].set_float(2.0f);
    vm.stack[1].set_float(5.0f);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);

    std::cout << func(vm.stack, &vm.functions[0]) << std::endl;

    res.set_nil();
    res.i32 += 1;
    temp = *reinterpret_cast<uint64_t *>(&res);
    vm.stack[0].set_float(2.0f);
    vm.stack[1].set_int(5.0f);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);

}

TEST(SimpleJitTest, TestMul) {
    VMData &vm = vm_instance();
    initVM();
    vm.ip = 0;
    vm.fp = 0;
    Value v;
    v.set_int(2);
    vm.constanti.push_back(v);
    v.set_int(3);
    vm.constanti.push_back(v);
    vm.code[vm.code_size++] = opcode(OP_MUL, 2, 0, 1);
    vm.code[vm.code_size++] = opcode(OP_RETURN, 2, 0, 0);
    vm.functions[0].code_size = vm.code_size;
    asmjit::JitRuntime jt;
    jit::JitRuntime rt;
    jit::FuncCompiled func;
    vm.functions[0].arity = 2;
    rt.compile(vm, vm.functions[0], func, vm.functions[0].arity);


    Value res;
    res.set_int(6);
    auto temp = *reinterpret_cast<uint64_t *>(&res);
    vm.stack[0].set_int(2.0f);
    vm.stack[1].set_int(3.0f);
//    return;
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);

    std::cout << func(vm.stack, &vm.functions[0]) << std::endl;

    res.set_float(6.0f);
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

TEST(SimpleJitTest, TestDiv) {
    VMData &vm = vm_instance();
    initVM();
    vm.ip = 0;
    vm.fp = 0;
    Value v;
    v.set_int(9);
    vm.constanti.push_back(v);
    v.set_int(3);
    vm.constanti.push_back(v);
    vm.code[vm.code_size++] = opcode(OP_DIV, 2, 0, 1);
    vm.code[vm.code_size++] = opcode(OP_RETURN, 2, 0, 0);
    vm.functions[0].code_size = vm.code_size;
    asmjit::JitRuntime jt;
    jit::JitRuntime rt;
    jit::FuncCompiled func;
    vm.functions[0].arity = 2;
    rt.compile(vm, vm.functions[0], func, vm.functions[0].arity);


    Value res;
    res.set_int(3);
    auto temp = *reinterpret_cast<uint64_t *>(&res);
    vm.stack[0].set_int(9.0f);
    vm.stack[1].set_int(3.0f);
//    return;
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);

    std::cout << func(vm.stack, &vm.functions[0]) << std::endl;

    res.set_float(3.0f);
    temp = *reinterpret_cast<uint64_t *>(&res);
    vm.stack[0].set_float(9.0f);
    vm.stack[1].set_float(3.0f);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);

    std::cout << func(vm.stack, &vm.functions[0]) << std::endl;

    res.set_nil();
    res.i32 += 1;
    temp = *reinterpret_cast<uint64_t *>(&res);
    vm.stack[0].set_float(9.0f);
    vm.stack[1].set_int(3.0f);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);

    res.set_nil();
    res.i32 += 1;
    temp = *reinterpret_cast<uint64_t *>(&res);
    vm.stack[0].set_float(9.0f);
    vm.stack[1].set_int(3.0f);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);

    res.set_float(std::numeric_limits<float>::infinity());
    temp = *reinterpret_cast<uint64_t *>(&res);
    vm.stack[0].set_float(9.0f);
    vm.stack[1].set_float(0.0f);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);

    res.set_nil();
    res.i32 += 1;
    temp = *reinterpret_cast<uint64_t *>(&res);
    vm.stack[0].set_int(9.0f);
    vm.stack[1].set_int(0.0f);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);


}

TEST(SimpleJitTest, TestModulo) {
    VMData &vm = vm_instance();
    initVM();
    vm.ip = 0;
    vm.fp = 0;
    Value v;
    v.set_int(8);
    vm.constanti.push_back(v);
    v.set_int(3);
    vm.constanti.push_back(v);
    vm.code[vm.code_size++] = opcode(OP_MOD, 2, 0, 1);
    vm.code[vm.code_size++] = opcode(OP_RETURN, 2, 0, 0);
    vm.functions[0].code_size = vm.code_size;
    asmjit::JitRuntime jt;
    jit::JitRuntime rt;
    jit::FuncCompiled func;
    vm.functions[0].arity = 2;
    rt.compile(vm, vm.functions[0], func, vm.functions[0].arity);


    Value res;
    res.set_int(2);
    auto temp = *reinterpret_cast<uint64_t *>(&res);
    vm.stack[0].set_int(8.0f);
    vm.stack[1].set_int(3.0f);
//    return;
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);

    std::cout << func(vm.stack, &vm.functions[0]) << std::endl;

    res.set_nil();
    res.i32 += 1;
    temp = *reinterpret_cast<uint64_t *>(&res);
    vm.stack[0].set_float(8.0f);
    vm.stack[1].set_int(3.0f);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);

    std::cout << func(vm.stack, &vm.functions[0]) << std::endl;

    res.set_nil();
    res.i32 += 1;
    temp = *reinterpret_cast<uint64_t *>(&res);
    vm.stack[0].set_float(8.0f);
    vm.stack[1].set_float(3.0f);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);

    res.set_nil();
    res.i32 += 1;
    temp = *reinterpret_cast<uint64_t *>(&res);
    vm.stack[0].set_int(9.0f);
    vm.stack[1].set_int(0.0f);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
    ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
}

using JitTestWithParam = TestWithParam<std::tuple<void (*)(BytecodeEmitter &), void (*)(Value *stack), Value>>;

TEST_P(JitTestWithParam, GenericTest) {
    auto p = GetParam();
    auto emitter = BytecodeEmitter();
    get<0>(p)(emitter);
    emitter.initVM(vm_instance());
    asmjit::JitRuntime jt;
    jit::JitRuntime rt;
    jit::FuncCompiled func;
    rt.compile(vm_instance(), vm_instance().functions[0], func, vm_instance().functions[0].arity);
    Value res = get<2>(p);
    auto temp = *reinterpret_cast<uint64_t *>(&res);
    get<1>(p)(vm_instance().stack);
    ASSERT_EQ(func(vm_instance().stack, &vm_instance().functions[0]), temp);

    std::cout << func(vm_instance().stack, &vm_instance().functions[0]) << std::endl;
}

inline Value fromInt(int x) {
    Value v;
    v.set_int(x);
    return v;
}

inline Value fromFloat(float x) {
    Value v;
    v.set_float(x);
    return v;
}

static constexpr uint64_t TEST_BAD_NIL = OBJ_NIL + 1ull;

INSTANTIATE_TEST_SUITE_P(
        GenericTest,
        JitTestWithParam,
        Values(make_tuple([](BytecodeEmitter &emitter) {
                   std::cout << "loadnil & return\n";
                   emitter.begin_func(0, "main");
                   emitter.emit_loadnil(0);
                   emitter.emit_return(0);
                   emitter.end_func();
               }, [](Value *stack) {
                   stack[0].set_nil();
                   stack[1].set_nil();
               }, *reinterpret_cast<const Value *>(&OBJ_NIL)),
               make_tuple([](BytecodeEmitter &emitter) {
                   std::cout << "loadint & ret\n";
                   emitter.begin_func(0, "main");
                   emitter.emit_loadi(0, 1);
                   emitter.emit_return(0);
                   emitter.end_func();
               }, [](Value *stack) {
                   stack[0].set_nil();
                   stack[1].set_nil();
               }, fromInt(1)),
               make_tuple([](BytecodeEmitter &emitter) {
                   std::cout << "jmp\n";
                   emitter.begin_func(0, "main");
                   emitter.emit_loadi(0, 1);
                   emitter.emit_loadi(1, 10);
                   emitter.jmp_label(0);
                   emitter.emit_retnil();
                   emitter.emit_return(1);
                   emitter.label(0);
                   emitter.emit_return(0);
                   emitter.end_func();
               }, [](Value *stack) {
                   stack[0].set_nil();
                   stack[1].set_nil();
               }, fromInt(1)),
               make_tuple([](BytecodeEmitter &emitter) {
                              std::cout << "mov\n";
                              emitter.begin_func(0, "main");
                              emitter.emit_loadi(0, 3);
                              emitter.emit_loadi(1, 9);
                              emitter.emit_move(1, 0);
                              emitter.emit_return(1);
                              emitter.end_func();
                          }, [](Value *stack) {
                              stack[0].set_nil();
                              stack[1].set_nil();
                          }, fromInt(3)
               ),
               make_tuple([](BytecodeEmitter &emitter) {
                              std::cout << "retnil\n";
                              emitter.begin_func(0, "main");
                              emitter.emit_retnil();
                              emitter.emit_return(1);
                              emitter.end_func();
                          }, [](Value *stack) {
                              stack[0].set_int(2);
                              stack[1].set_int(2);
                          }, *reinterpret_cast<const Value *>(&OBJ_NIL)
               ),
               make_tuple([](BytecodeEmitter &emitter) {
                              std::cout << "neg int\n";
                              emitter.begin_func(0, "main");
                              emitter.emit_loadi(0, 0);
                              emitter.emit_neg(0, 1);
                              emitter.emit_return(0);
                              emitter.end_func();
                          }, [](Value *stack) {
                              stack[0].set_int(2);
                              stack[1].set_int(2);
                          }, fromInt(-2)
               ),
               make_tuple([](BytecodeEmitter &emitter) {
                              std::cout << "neg float\n";
                              emitter.begin_func(0, "main");
                              emitter.emit_loadi(0, 0);
                              emitter.emit_neg(0, 1);
                              emitter.emit_return(0);
                              emitter.end_func();
                          }, [](Value *stack) {
                              stack[0].set_int(2);
                              stack[1].set_float(2.5f);
                          }, fromFloat(-2.5f)
               ),
               make_tuple([](BytecodeEmitter &emitter) {
                              std::cout << "load float\n";
                              emitter.begin_func(0, "main");
                              emitter.emit_loadf(0, -1.5f);
                              emitter.emit_return(0);
                              emitter.end_func();
                          }, [](Value *stack) {
                              stack[0].set_int(2);
                              stack[1].set_int(2);
                          }, fromFloat(-1.5f)
               ),
               make_tuple([](BytecodeEmitter &emitter) {
                              std::cout << "load eq(true)\n";
                              emitter.begin_func(0, "main");
                              emitter.emit_eq(2, 0, 1);
                              emitter.emit_return(2);
                              emitter.end_func();
                          }, [](Value *stack) {
                              stack[0].set_int(2);
                              stack[1].set_int(2);
                          }, fromInt(1)
               ),
               make_tuple([](BytecodeEmitter &emitter) {
                              std::cout << "load eq(false)\n";
                              emitter.begin_func(0, "main");
                              emitter.emit_eq(2, 0, 1);
                              emitter.emit_return(2);
                              emitter.end_func();
                          }, [](Value *stack) {
                              stack[0].set_int(2);
                              stack[1].set_int(3);
                          }, fromInt(0)
               ),
               make_tuple([](BytecodeEmitter &emitter) {
                              std::cout << "load eq(true, float)\n";
                              emitter.begin_func(0, "main");
                              emitter.emit_eq(2, 0, 1);
                              emitter.emit_return(2);
                              emitter.end_func();
                          }, [](Value *stack) {
                              stack[0].set_float(2.0f);
                              stack[1].set_float(2.0f);
                          }, fromInt(1)
               ),

               make_tuple([](BytecodeEmitter &emitter) {
                              std::cout << "load less(true, float)\n";
                              emitter.begin_func(0, "main");
                              emitter.emit_less(2, 0, 1);
                              emitter.emit_return(2);
                              emitter.end_func();
                          }, [](Value *stack) {
                              stack[0].set_float(1.0f);
                              stack[1].set_float(2.0f);
                          }, fromInt(0)
               ),
               make_tuple([](BytecodeEmitter &emitter) {
                              std::cout << "load less(false, float, equal)\n";
                              emitter.begin_func(0, "main");
                              emitter.emit_less(2, 0, 1);
                              emitter.emit_return(2);
                              emitter.end_func();
                          }, [](Value *stack) {
                              stack[0].set_float(2.0f);
                              stack[1].set_float(2.0f);
                          }, fromInt(0)
               ),

               make_tuple([](BytecodeEmitter &emitter) {
                              std::cout << "load less(false, float, not equal)\n";
                              emitter.begin_func(0, "main");
                              emitter.emit_less(2, 0, 1);
                              emitter.emit_return(2);
                              emitter.end_func();
                          }, [](Value *stack) {
                              stack[0].set_float(3.0f);
                              stack[1].set_float(2.0f);
                          }, fromInt(0)
               ),
               make_tuple([](BytecodeEmitter &emitter) {
                              std::cout << "load less(true, int)\n";
                              emitter.begin_func(0, "main");
                              emitter.emit_less(2, 0, 1);
                              emitter.emit_return(2);
                              emitter.end_func();
                          }, [](Value *stack) {
                              stack[0].set_int(1.0f);
                              stack[1].set_int(2.0f);
                          }, fromInt(1)
               ),

               make_tuple([](BytecodeEmitter &emitter) {
                              std::cout << "load less(false, int, equal)\n";
                              emitter.begin_func(0, "main");
                              emitter.emit_less(2, 0, 1);
                              emitter.emit_return(2);
                              emitter.end_func();
                          }, [](Value *stack) {
                              stack[0].set_int(2);
                              stack[1].set_int(2);
                          }, fromInt(0)
               ),

               make_tuple([](BytecodeEmitter &emitter) {
                              std::cout << "load less(false, int, not equal)\n";
                              emitter.begin_func(0, "main");
                              emitter.emit_less(2, 0, 1);
                              emitter.emit_return(2);
                              emitter.end_func();
                          }, [](Value *stack) {
                              stack[0].set_int(3.0f);
                              stack[1].set_int(2.0f);
                          }, fromInt(0)
               ),
               make_tuple([](BytecodeEmitter &emitter) {
                              std::cout << "load less(fail, different types)\n";
                              emitter.begin_func(0, "main");
                              emitter.emit_less(2, 0, 1);
                              emitter.emit_return(2);
                              emitter.end_func();
                          }, [](Value *stack) {
                              stack[0].set_float(3.0f);
                              stack[1].set_int(2.0f);
                          }, *reinterpret_cast<const Value *>(&TEST_BAD_NIL)
               ),
               make_tuple([](BytecodeEmitter &emitter) {
                              std::cout << "load leq(true, float)\n";
                              emitter.begin_func(0, "main");
                              emitter.emit_leq(2, 0, 1);
                              emitter.emit_return(2);
                              emitter.end_func();
                          }, [](Value *stack) {
                              stack[0].set_float(1.0f);
                              stack[1].set_float(2.0f);
                          }, fromInt(0)
               ),
               make_tuple([](BytecodeEmitter &emitter) {
                              std::cout << "load leq(true, float, equal)\n";
                              emitter.begin_func(0, "main");
                              emitter.emit_leq(2, 0, 1);
                              emitter.emit_return(2);
                              emitter.end_func();
                          }, [](Value *stack) {
                              stack[0].set_float(2.0f);
                              stack[1].set_float(2.0f);
                          }, fromInt(1)
               ),

               make_tuple([](BytecodeEmitter &emitter) {
                              std::cout << "load leq(false, float, not equal)\n";
                              emitter.begin_func(0, "main");
                              emitter.emit_leq(2, 0, 1);
                              emitter.emit_return(2);
                              emitter.end_func();
                          }, [](Value *stack) {
                              stack[0].set_float(3.0f);
                              stack[1].set_float(2.0f);
                          }, fromInt(0)
               ),
               make_tuple([](BytecodeEmitter &emitter) {
                              std::cout << "load leq(true, int)\n";
                              emitter.begin_func(0, "main");
                              emitter.emit_leq(2, 0, 1);
                              emitter.emit_return(2);
                              emitter.end_func();
                          }, [](Value *stack) {
                              stack[0].set_int(1.0f);
                              stack[1].set_int(2.0f);
                          }, fromInt(1)
               ),

               make_tuple([](BytecodeEmitter &emitter) {
                              std::cout << "load leq(true, int, equal)\n";
                              emitter.begin_func(0, "main");
                              emitter.emit_leq(2, 0, 1);
                              emitter.emit_return(2);
                              emitter.end_func();
                          }, [](Value *stack) {
                              stack[0].set_int(2);
                              stack[1].set_int(2);
                          }, fromInt(1)
               ),

               make_tuple([](BytecodeEmitter &emitter) {
                              std::cout << "load leq(false, int, not equal)\n";
                              emitter.begin_func(0, "main");
                              emitter.emit_leq(2, 0, 1);
                              emitter.emit_return(2);
                              emitter.end_func();
                          }, [](Value *stack) {
                              stack[0].set_int(3.0f);
                              stack[1].set_int(2.0f);
                          }, fromInt(0)
               ),
               make_tuple([](BytecodeEmitter &emitter) {
                              std::cout << "load leq(fail, different types)\n";
                              emitter.begin_func(0, "main");
                              emitter.emit_leq(2, 0, 1);
                              emitter.emit_return(2);
                              emitter.end_func();
                          }, [](Value *stack) {
                              stack[0].set_float(3.0f);
                              stack[1].set_int(2.0f);
                          }, *reinterpret_cast<const Value *>(&TEST_BAD_NIL)
               )
        )
);

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