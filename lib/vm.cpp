//
// Created by motya on 01.04.2025.
//

#include "vm.h"
#include "mish.h"

#define DISPATCH() workers[fetch()]()
#define DISPATCH_FETCHED() workers[vm.cur]()
namespace {
    using namespace interpreter;
    constexpr uint32_t low_8bits = (1 << 8) - 1;
    constexpr uint32_t low_16bits = (1 << 16) - 1;
    constexpr uint32_t high_8bits = low_8bits << 24;
    constexpr uint32_t high_16bits = low_8bits << 16;

    typedef void(*service_f)();

    interpreter::VMData vm;
    void *m_stream_ptr = &vm;

    void op_todo() { throw std::runtime_error("operation " + std::to_string(vm.cur) + " not implemented"); }

    void op_stop() {}

    void op_push_constant();

    void op_addi();

    void op_subi() { op_todo(); }

    service_f workers[] = {
            op_stop,
            op_todo, op_todo, op_todo, op_push_constant,
            op_todo, op_todo, op_addi, op_subi
    };

    byte fetch() { return vm.code[vm.ip++]; }

    uint16_t fetch2() { return fetch() + (fetch() << 8u);/* or the other way around */ }

    Value &peek(int offset = 0) { return vm.stack[vm.sp - offset - 1]; }

    void run_program() {
        DISPATCH();
    }

    void op_push_constant() {
        vm.stack[vm.sp++] = vm.constant_pool[fetch()];
        DISPATCH();
    }

    void op_addi() {
        vm.sp--;
        if (!(peek(0).mtype == ValueType::Int && peek(-1).mtype == ValueType::Int)) throw std::runtime_error("ERROR");
        peek(0).mdata.asInt = peek(-1).mdata.asInt + peek(0).mdata.asInt;
        DISPATCH();
    }


}
namespace interpreter {
    void example() {
        vm.constant_pool[0] = Value(ValueType::Int, 3);
        vm.constant_pool[1] = Value(ValueType::Int, 2);
        vm.code[0] = OP_CONSTANT;
        vm.code[1] = 0;
        vm.code[2] = OP_CONSTANT;
        vm.code[3] = 1;
        vm.code[4] = OP_ADDI;
        run_program();
        if (!(peek(0).mtype == ValueType::Int && peek(0).mdata.asInt == 5)) {
            throw std::runtime_error("oops");
        }
    }

    void run() {
        run_program();
    }
}