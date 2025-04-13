//
// Created by motya on 01.04.2025.
//

#include "vm.h"

#include <cstdint>
#include <cstdio>
#include <stdexcept>

#include "misc.h"

#define DISPATCH() workers[fetch() & low_6bits]()
#define DISPATCH_FETCHED() workers[vm.cur]()

#define J ((int32_t)(vm.cur >> interpreter::OPCODE_SIZE))
#define C() ((vm.cur & argC) >> interpreter::OPCODE_SIZE)
#define B() ((vm.cur & argB) >> (interpreter::OPCODE_SIZE + interpreter::SIZE_C))
#define A() ((vm.cur & argA) >> (interpreter::OPCODE_SIZE + interpreter::SIZE_C + interpreter::SIZE_B))

namespace {
    using namespace interpreter;

    constexpr uint32_t low_6bits = (1 << 6) - 1;

    constexpr uint32_t argC = ((1 << SIZE_C) - 1) << OPCODE_SIZE;
    constexpr uint32_t argB = ((1 << SIZE_B) - 1) << (SIZE_C + OPCODE_SIZE);
    constexpr uint32_t argA = ((1 << SIZE_A) - 1) << (SIZE_B + SIZE_C + OPCODE_SIZE);
    constexpr uint32_t argAX = ((1 << SIZE_AX) - 1) << OPCODE_SIZE;
    constexpr uint32_t argJ = ((1 << SIZE_J) - 1) << OPCODE_SIZE;
    constexpr uint32_t J_shift = ((1 << SIZE_J) - 1) >> 1;

    typedef void (*service_f)();

    interpreter::VMData vm;

    void op_todo() {
        throw std::runtime_error("operation " + std::to_string(vm.cur) + " not implemented");
    }

    void op_stop() {}

    void op_return() {
        op_todo();
    }

    void op_end() {
        op_todo();
    }

    void op_push_constant();

    void op_addi();

    void op_subi();

    void op_multi();

    void op_divi();

    void op_negate();

    void op_jmpne();

    void op_jmpeq();

    void op_jmpge();

    void op_jmpgt();

    void op_jmple();

    void op_jmplt();

    void op_jmp();

    void op_jmpt();

    service_f workers[] = {
            op_stop, op_end, op_return,                                 // Code block endings
            op_push_constant, op_todo, op_todo,                                   // Pushes to stack
            op_addi, op_subi, op_divi, op_multi, op_negate,            // In stack arithmetics
            op_jmpne, op_jmpeq, op_jmplt, op_jmple, op_jmpgt, op_jmpge,  // Jumps
            op_jmp, op_jmpt                                              // Jumps2
    };

    uint32_t fetch() {
        // printf("op code: %i\n", vm.code[vm.ip] & low_6bits);
        return vm.cur = vm.code[vm.ip++];
    }

    Value &peek(int offset = 0) {
        return vm.stack[vm.sp - offset - 1];
    }

    std::pair<Value, Value> pop_two() {
        vm.sp -= 2;
        if (vm.sp < 0) {
            throw std::runtime_error("tried to pop from empty stack");
        }
        return std::make_pair<>(peek(-1), peek(-2));
    }

    void run_program() {
        DISPATCH();
    }

    void op_push_constant() {
        vm.stack[vm.sp++] = vm.constant_pool[C()];
        DISPATCH();
    }

    void op_addi() {
        vm.sp--;
        if (!(peek(0).mtype == ValueType::Int && peek(-1).mtype == ValueType::Int))
            throw std::runtime_error("ERROR");
        peek(0).mdata.asInt = peek(-1).mdata.asInt + peek(0).mdata.asInt;
        DISPATCH();
    }

    void op_subi() {
        vm.sp--;
        if (!(peek(0).mtype == ValueType::Int && peek(-1).mtype == ValueType::Int))
            throw std::runtime_error("ERROR");
        peek(0).mdata.asInt = peek(-1).mdata.asInt - peek(0).mdata.asInt;
        DISPATCH();
    }

    void op_multi() {
        vm.sp--;
        if (!(peek(0).mtype == ValueType::Int && peek(-1).mtype == ValueType::Int))
            throw std::runtime_error("ERROR");
        peek(0).mdata.asInt = peek(-1).mdata.asInt * peek(0).mdata.asInt;
        DISPATCH();
    }

    void op_divi() {
        vm.sp--;
        if (!(peek(0).mtype == ValueType::Int && peek(-1).mtype == ValueType::Int))
            throw std::runtime_error("ERROR");
        else if (peek().mdata.asInt == 0)
            throw std::runtime_error("Division by zero");
        peek(0).mdata.asInt = peek(-1).mdata.asInt / peek(0).mdata.asInt;
        DISPATCH();
    }

    void op_negate() {
        peek().mdata.asInt = -peek().mdata.asInt;
        DISPATCH();
    }

    void op_jmpne() {
        std::pair<Value, Value> popped = pop_two();
        vm.ip += J * int(nequal_val(popped.first, popped.second));
        DISPATCH();
    }

    void op_jmpeq() {
        std::pair<Value, Value> popped = pop_two();
        vm.ip += J * int(equal_val(popped.first, popped.second));
        DISPATCH();
    }

    void op_jmplt() {
        std::pair<Value, Value> popped = pop_two();
        vm.ip += J * int(less(popped.first, popped.second));
        DISPATCH();
    }

    void op_jmple() {
        std::pair<Value, Value> popped = pop_two();
        vm.ip += J * int(less_equal(popped.first, popped.second));
        DISPATCH();
    }

    void op_jmpge() {
        std::pair<Value, Value> popped = pop_two();
        vm.ip += J * int(greater_equal(popped.first, popped.second));
        DISPATCH();
    }

    void op_jmpgt() {
        std::pair<Value, Value> popped = pop_two();
        vm.ip += J * int(greater(popped.first, popped.second));
        DISPATCH();
    }

    void op_jmp() {
        vm.ip += J;
    }

    void op_jmpt() {
        vm.sp--;
        vm.ip += J * (peek(-1).mdata.asInt != 0);
    }
}  // namespace

namespace interpreter {

    VMData& vm_instance()  { return vm; }

    void run() {
        run_program();
    }

    bool equal_val(Value val1, Value val2) {
        if (val1.mtype != val2.mtype)
            return false;

        switch (val1.mtype) {
            case ValueType::Char:
            case ValueType::Int:
                return val1.mdata.asInt == val2.mdata.asInt;
            case ValueType::Double:
                return val1.mdata.asDouble == val2.mdata.asDouble;
        }
    }

    bool nequal_val(Value val1, Value val2) {
        return !equal_val(val1, val2);
    }

    bool less(Value val1, Value val2) {
        if (val1.mtype != val2.mtype)
            throw std::runtime_error("Calling less on different types");

        switch (val1.mtype) {
            case ValueType::Char:
            case ValueType::Int:
                return val1.mdata.asInt < val2.mdata.asInt;
            case ValueType::Double:
                return val1.mdata.asDouble < val2.mdata.asDouble;
        }
    }

    bool less_equal(Value val1, Value val2) {
        if (val1.mtype != val2.mtype)
            throw std::runtime_error("Calling less or equal on different types");

        switch (val1.mtype) {
            case ValueType::Char:
            case ValueType::Int:
                return val1.mdata.asInt <= val2.mdata.asInt;
            case ValueType::Double:
                return val1.mdata.asDouble <= val2.mdata.asDouble;
        }
    }

    bool greater(Value val1, Value val2) {
        if (val1.mtype != val2.mtype)
            throw std::runtime_error("Calling greater on different types");

        switch (val1.mtype) {
            case ValueType::Char:
            case ValueType::Int:
                return val1.mdata.asInt > val2.mdata.asInt;
            case ValueType::Double:

                return val1.mdata.asDouble > val2.mdata.asDouble;
        }
    }

    bool greater_equal(Value val1, Value val2) {
        if (val1.mtype != val2.mtype)
            throw std::runtime_error("Calling greater on different types");

        switch (val1.mtype) {
            case ValueType::Char:
            case ValueType::Int:
                return val1.mdata.asInt >= val2.mdata.asInt;
            case ValueType::Double:
                return val1.mdata.asDouble >= val2.mdata.asDouble;
        }
    }

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

    void jmp_example() {
        vm.constant_pool[0] = Value(ValueType::Int, 3);
        vm.constant_pool[1] = Value(ValueType::Int, 2);
        vm.constant_pool[2] = Value(ValueType::Int, 100000);
        vm.constant_pool[3] = Value(ValueType::Int, 5);
        vm.code[0] = OP_CONSTANT + 0;                            // 3
        vm.code[1] = OP_CONSTANT + (1 << OPCODE_SIZE);           // 2
        vm.code[2] = OP_CONSTANT + (3 << OPCODE_SIZE);           // 5
        vm.code[3] = OP_JMPNE + ((1 + J_shift) << OPCODE_SIZE);  // skip next operation
        vm.code[4] = OP_CONSTANT + (2 << OPCODE_SIZE);           // if not skipped pulls big number into stack
        vm.code[5] = OP_CONSTANT + (1 << OPCODE_SIZE);           // 2
        vm.code[6] = OP_ADDI;                                    // expected to add 3 and 2
        run_program();

        if (!(peek().mtype == ValueType::Int && peek().mdata.asInt == 5)) {
            throw std::runtime_error("jump example failed");
        }
    }

}  // namespace interpreter
