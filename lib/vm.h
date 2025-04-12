//
// Created by motya on 01.04.2025.
//

#ifndef CRYPT_VM_H
#define CRYPT_VM_H

#include <cstdint>
#include <fstream>

namespace interpreter {
using byte = uint8_t;

// supports: int, double, {char, objects, array, null}?
// something like this: https://www.lua.org/source/4.0/lopcodes.h.html
//  or like this: https://github.com/munificent/craftinginterpreters/blob/master/c/chunk.h
//  each instruction is 1 byte
enum OpCode {

    //== Code block endings

    OP_STOP,
    OP_END,     // return from void function
    OP_RETURN,  // returns from function

    //== Pushes to stack

    OP_CONSTANT,   // C -> pushes constant on the stack
    OP_GET_LOCAL,  // pushes local var value TODO:
    OP_SET_LOCAL,  // set local var value from stack TODO:

    //== In stack arithmetics

    OP_ADDI,
    OP_SUBI,
    OP_DIVI,
    OP_MULI,
    OP_NEGATE,
    /*
            OP_ADDD,
            OP_SUBD,
            OP_DIVD,
            OP_MULD,
            not needed right now
    */

    //== Jumps

    // does pc += J if predicate on popped from stack is true
    OP_JMPNE,  // J -> not equal
    OP_JMPEQ,  // J -> equal
    OP_JMPLT,  // J -> less
    OP_JMPLE,  // J -> less or equals
    OP_JMPGT,  // J -> greater
    OP_JMPGE,  // J -> greater or equal

    OP_JMP,   // J -> does pc += J
    OP_JMPT,  // J -> does pc += J if popped != 0
    // OP_JMPF,//jump if false(or null)??? false == null????

    // TODO: clojures, classes, objects, arrays; (for, while - more specific)

};
enum class ValueType : uint8_t {
    Int,
    Double,
    Char,
};
union ValueData {
    int64_t asInt;
    double asDouble;
    ValueData(int64_t asInt) : asInt(asInt) {}
    // TODO: obj, array,...
};
struct Value {
    ValueType mtype;
    ValueData mdata;
    Value(ValueType mtype, ValueData mdata) : mtype(mtype), mdata(mdata) {}
    Value() : Value(ValueType::Int, 1) {}
};
// Codes and arguments sizes
static constexpr int OPCODE_SIZE = 6;
static constexpr int SIZE_C      = 9;
static constexpr int SIZE_B      = 9;
static constexpr int SIZE_A      = 8;
static constexpr int SIZE_AX     = 26;
static constexpr int SIZE_J      = 18;

// Internal interpreter sizes
static constexpr int MAX_CONSTANTS      = 1 << 8;
static constexpr int PROGRAM_STACK_SIZE = 1024;
struct VMData {
    Value constant_pool[MAX_CONSTANTS];
    int constant_count = 0;
    uint32_t ip = 0, sp = 0;
    uint32_t cur = 0;
    uint32_t code[1024];  // actually will contain list of function - each has a pointer to a block of code

    Value stack[PROGRAM_STACK_SIZE];
};

bool less(Value val1, Value val2);
bool less_equal(Value val1, Value val2);
bool equal_val(Value val1, Value val2);
bool nequal_val(Value val1, Value val2);
bool greater(Value val1, Value val2);
bool greater_equal(Value val1, Value val2);

void run();

void example();
void jmp_example();

// inits from file(or any other stream); TODO (example - jvm)
void init_vm(std::istream& in);
};  // namespace interpreter

#endif  // CRYPT_VM_H
