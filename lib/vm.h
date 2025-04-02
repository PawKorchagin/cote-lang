//
// Created by motya on 01.04.2025.
//

#ifndef CRYPT_VM_H
#define CRYPT_VM_H

#include <fstream>
#include <cstdint>


namespace interpreter {
    using byte = uint8_t;

    //supports: int, double, {char, objects, array, null}?
    //something like this: https://www.lua.org/source/4.0/lopcodes.h.html
    // or like this: https://github.com/munificent/craftinginterpreters/blob/master/c/chunk.h
    // each instruction is 1 byte
    enum OpCode {
        OP_STOP,
        OP_END,//return from void function
        OP_RETURN,//returns from function

        OP_CALL,//?
        OP_CONSTANT,//pushes constant on the stack
        OP_GET_LOCAL,//pushes local var value
        OP_SET_LOCAL,//set local var value from stack

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

        OP_JMPNE, // J -> pops two vars from stack and if they are not equal does: pc+=J
        OP_JMPEQ,
        OP_JMPLT,//less
        OP_JMPLE,//less or equals
        OP_JMPGT,
        OP_JMPGE,

        OP_JMP,//unconditional jump
        OP_JMPT,//jump if true aka != 0 for now
        //OP_JMPF,//jump if false(or null)??? false == null????


        //TODO: clojures, classes, objects, arrays; (for, while - more specific)

    };
    enum class ValueType : uint8_t {
        Int,
        Double,
        Char,
    };
    union ValueData {
        int64_t asInt;
        double asDouble;
        ValueData(int64_t asInt): asInt(asInt) {}
        //TODO: obj, array,...
    };
    struct Value {
        ValueType mtype;
        ValueData mdata;
        Value(ValueType mtype, ValueData mdata):mtype(mtype), mdata(mdata) {}
        Value(): Value(ValueType::Int, 1) {}
    };
    static constexpr int MAX_CONSTANTS = 1 << 8;
    static constexpr int OPCODE_SIZE = 8;
    static constexpr int PROGRAM_STACK_SIZE = 1024;
    struct VMData {
        Value constant_pool[MAX_CONSTANTS];
        int constant_count = 0;
        uint32_t ip = 0, sp = 0;
        byte cur = 0;
        byte code[1024];//actually will contain list of function - each has a pointer to a block of code

        Value stack[PROGRAM_STACK_SIZE];
    };

    void run();

    void example();
    //inits from file(or any other stream); TODO (example - jvm)
    void init_vm(std::istream& in);
};

#endif //CRYPT_VM_H
