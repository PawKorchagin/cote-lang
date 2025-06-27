//
// Created by motya on 27.06.2025.
//
#include "utils.h"
#include "src/ast.h"
#include "src/jit_runtime.h"
#include "libs/asmjit/src/asmjit/x86.h"
#include "lang_stdlib.h"

using namespace asmjit;
using namespace interpreter;

void test_jit1(std::istream &fin, Value res) {
    auto &vm = initVM();
    parser::init_parser(fin, new BytecodeEmitter());
    ASSERT_NO_THROW(parser::parse_program(vm));
    print_vm_data(vm);
    for (int i = 0; i < vm.functions_count; ++i) {
        vm.functions[i].hotness = 100;
    }
    vm.jit_log_level = 1;
    interpreter::run();
    ASSERT_EQ(*reinterpret_cast<const uint64_t *>(&vm_instance().stack[0]), *reinterpret_cast<uint64_t *>(&res));

}

BytecodeEmitter *test_jit_compile(std::string filename) {
    std::ifstream fin(filename);
    auto &vm = initVM();
    auto emitter = new BytecodeEmitter();
    parser::init_parser(fin, emitter);
    parser::parse_program(vm);
    vm.jit_log_level = 0;
    return emitter;
}

TEST(ProgramJitTest, TestArrSet) {
    std::ifstream fin("../../tests/sources/arrset1.ct");
    Value v;
    v.set_int(10);
    test_jit1(fin, v);
    //res
//    auto temp = *reinterpret_cast<uint64_t *>(&res);
    //set stack
}


TEST(ProgramJitTest, TestArrGet) {
    std::ifstream fin("../../tests/sources/arrget1.ct");
    Value v;
    v.set_int(1);
    test_jit1(fin, v);
    //res
//    auto temp = *reinterpret_cast<uint64_t *>(&res);
    //set stack
}

TEST(ProgramJitTest, Test2) {
    std::ifstream tempf("any.txt");
    auto emitter = BytecodeEmitter();
    parser::init_parser(tempf, &emitter);

    emitter.begin_func(1, "main");

    emitter.emit_mod(1, 1, 2);
    emitter.emit_eq(1, 1, 3);
    emitter.emit_return(1);

    emitter.end_func();

    emitter.initVM(vm_instance());
    asmjit::JitRuntime jt;
    jit::JitRuntime rt;
    jit::FuncCompiled func;
    rt.compile(vm_instance(), vm_instance().functions[0], func);
    Value res;
    res.set_int(0);
    auto temp = *reinterpret_cast<uint64_t *>(&res);
    vm_instance().stack[1].set_int(5);
    vm_instance().stack[2].set_int(2);
    vm_instance().stack[3].set_int(0);
    ASSERT_EQ(func(vm_instance().stack), temp);
}

using PerfomanceJitOnAndOff = Test;

template<typename T>
auto measure1(T func) {
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::microseconds;

    auto t1 = high_resolution_clock::now();
    func();
    auto t2 = high_resolution_clock::now();

    /* Getting number of milliseconds as an integer. */
    return duration_cast<microseconds>(t2 - t1).count();
}

void simple_perfomance_cmp(std::string filename) {
    interpreter::set_jit_off();
    auto emitter = test_jit_compile(filename);
    //Warm up 1
    std::cout << "Warm up 1\n";
    emitter->initVM(vm_instance());
    interpreter::run();
    std::cout << "Warm up 2\n";
    emitter->initVM(vm_instance());
    interpreter::run();
    std::cout << "Jit off\n";
    emitter->initVM(vm_instance());
    auto y = measure1([]() {
        interpreter::run();
    });
    std::cout << "Time: " << y / 1000 << '.' << y % 1000 << std::endl;
    std::cout << "Jit on\n";
    interpreter::set_jit_on();
//    print_vm_data(vm_instance());
//    interpreter::vm_instance().jit_log_level = 2;
    emitter->initVM(vm_instance());
    for (int i = 0; i < vm_instance().functions_count; i++) {
        vm_instance().functions[i].hotness = 100;
    }
    auto x = measure1([]() {
        interpreter::run();
    });
    std::cout << "Time: " << x / 1000 << '.' << x % 1000 << std::endl;
    std::cout << "Results: Jit off:  " << y / 1000 << '.' << y % 1000 << std::endl;
    std::cout << "         Jit on:  " << x / 1000 << '.' << x % 1000 << std::endl;
}

TEST(PerfomanceJitOnAndOff, Test1_StupidCalculation) {
    simple_perfomance_cmp("../../tests/sources/jitSimple2.ct");
}

TEST(PerfomanceJitOnAndOff, TestPrimes) {
    simple_perfomance_cmp("../../tests/sources/test4.ct");
}

TEST(PerfomanceJitOnAndOff, TestEvenOdd) {
    simple_perfomance_cmp("../../tests/sources/jitSimple.ct");
}

TEST(PerfomanceJitOnAndOff, QuadraticSort) {
    simple_perfomance_cmp("../../tests/sources/test_sort1.ct");
}

TEST(PerfomanceJitOnAndOff, Playground) {
    simple_perfomance_cmp("../../tests/sources/jitPlayground.ct");
}