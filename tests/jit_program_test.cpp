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
//    print_vm_data(vm_instance());
    std::cout << "Jit on\n";
    interpreter::set_jit_on();
    interpreter::vm_instance().jit_log_level = 1;
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

TEST(PerfomanceJitOnAndOff, Sort) {
    simple_perfomance_cmp("../../tests/sources/test_quicksort.ct");
}

TEST(PerfomanceJitOnAndOff, Playground) {
    simple_perfomance_cmp("../../tests/sources/jitPlayground.ct");
}
/*
 constants: [2, 1, 0, 10]
func0(args: 3):
    mov [3] [1]
    mov [4] [2]
    leq [3] [4] [3]
    jmpf [3] 2
    mov [3] nil
    ret [3]
    mov [3] [0]
    mov [4] [1]
    mov [5] [2]
    add [4] [4] [5]
    mov [5] 2
    div [4] [4] [5]
    arrget [3] [3][4]
    mov [4] [1]
    mov [5] [2]
    mov [6] [4]
    mov [7] [5]
    leq [6] [6] [7]
    jmpf [6] 54
    mov [6] [0]
    mov [7] [4]
    arrget [6] [6][7]
    mov [7] [3]
    lt [6] [6] [7]
    jmpf [6] 8
    mov [6] 1
    add [4] [6] [4]
    mov [6] [0]
    mov [7] [4]
    arrget [6] [6][7]
    mov [7] [3]
    lt [6] [6] [7]
    jmpt [6] -8
    mov [6] [0]
    mov [7] [5]
    arrget [6] [6][7]
    mov [7] [3]
    lt [6] [7] [6]
    jmpf [6] 8
    mov [6] 1
    sub [5] [5] [6]
    mov [6] [0]
    mov [7] [5]
    arrget [6] [6][7]
    mov [7] [3]
    lt [6] [7] [6]
    jmpt [6] -8
    mov [6] [4]
    mov [7] [5]
    leq [6] [7] [6]
    jmpf [6] 1
    jmp 21
    mov [6] [0]
    mov [7] [4]
    arrget [6] [6][7]
    mov [7] [0]
    mov [8] [5]
    arrget [7] [7][8]
    mov [8] [0]
    mov [9] [4]
    arrset [8][9] [7]
    mov [7] [6]
    mov [8] [0]
    mov [9] [5]
    arrset [8][9] [7]
    mov [7] 1
    add [4] [7] [4]
    mov [7] 1
    sub [5] [5] [7]
    mov [7] [4]
    mov [8] [5]
    leq [7] [7] [8]
    jmpt [7] -54
    mov [6] f[0]
    mov [7] [0]
    mov [8] 0
    mov [9] [5]
    mov [10] 1
    sub [9] [9] [10]
    invoke [6] [7]...[9]
    mov [6] [7]
    mov [6] f[0]
    mov [7] [0]
    mov [8] [5]
    mov [9] 1
    add [8] [8] [9]
    mov [10] [0]
    native [3] [10]...[10]
    mov [9] [10]
    mov [10] 1
    sub [9] [9] [10]
    invoke [6] [7]...[9]
    mov [6] [7]
    ret nil
func1(args: 0):
    mov [0] 10
    mov [2] [0]
    alloc [1] [2]
    mov [2] 0
    mov [3] [2]
    mov [4] [0]
    lt [3] [3] [4]
    jmpf [3] 11
    native [4] [4]...[3]
    mov [3] [4]
    mov [4] [1]
    mov [5] [2]
    arrset [4][5] [3]
    mov [3] 1
    add [2] [3] [2]
    mov [3] [2]
    mov [4] [0]
    lt [3] [3] [4]
    jmpt [3] -11
    mov [2] f[0]
    mov [3] [1]
    mov [4] 0
    mov [6] [1]
    native [3] [6]...[6]
    mov [5] [6]
    mov [6] 1
    sub [5] [5] [6]
    invoke [2] [3]...[5]
    mov [2] [3]
    mov [2] 0
    mov [3] 1
    mov [4] [3]
    mov [5] [0]
    lt [4] [4] [5]
    jmpf [4] 16
    mov [4] [1]
    mov [5] [3]
    mov [6] 1
    sub [5] [5] [6]
    arrget [4] [4][5]
    mov [5] [1]
    mov [6] [3]
    arrget [5] [5][6]
    leq [4] [4] [5]
    add [2] [4] [2]
    mov [4] 1
    add [3] [4] [3]
    mov [4] [3]
    mov [5] [0]
    lt [4] [4] [5]
    jmpt [4] -16
    mov [3] [2]
    mov [4] [0]
    sub [3] [3] [4]
    mov [4] 1
    add [3] [3] [4]
    jmpf [3] 2
    native [5] [4]...[3]
    mov [3] [4]
    ret nil
    call f1 [0]...[0] <- ip
    halt
    call f1 [0]...[0]
    halt
 */