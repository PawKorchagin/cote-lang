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

void test_jit1(std::istream &fin, int id) {
    auto &vm = initVM();
    parser::init_parser(fin, new BytecodeEmitter());
    ASSERT_NO_THROW(parser::parse_program(vm));
    print_vm_data(vm);
    for (int i = 0; i < vm.functions_count; ++i) {
        vm.functions[i].hotness = 100;
    }
    vm.jit_log_level = 1;
    interpreter::run();
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

TEST(ProgramJitTest, Test1) {
    std::ifstream fin("../../tests/sources/jitSimple2.ct");
    test_jit1(fin, 0);
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
    using std::chrono::milliseconds;

    auto t1 = high_resolution_clock::now();
    func();
    auto t2 = high_resolution_clock::now();

    /* Getting number of milliseconds as an integer. */
    return duration_cast<milliseconds>(t2 - t1).count();
}

TEST(PerfomanceJitOnAndOff, Test1) {
    interpreter::set_jit_off();
    auto emitter = test_jit_compile("../../tests/sources/jitSimple2.ct");
    //Warm up 1
    std::cout << "Warm up 1\n";
    emitter->initVM(vm_instance());
    interpreter::run();
    std::cout << "Warm up 2\n";
    emitter->initVM(vm_instance());
    interpreter::run();
    std::cout << "Warm up 3\n";
    emitter->initVM(vm_instance());
    interpreter::run();
    std::cout << "Jit off\n";
    emitter->initVM(vm_instance());
    std::cout << "Time: " << measure1([]() {
        interpreter::run();
    }) << std::endl;
    std::cout << "Jit on\n";
    interpreter::set_jit_on();
    emitter->initVM(vm_instance());
    std::cout << "Time: " << measure1([]() {
        interpreter::run();
    }) << std::endl;
}