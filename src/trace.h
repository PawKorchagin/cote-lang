//
// Created by motya on 20.06.2025.
//

#ifndef CRYPT_TRACE_H
#define CRYPT_TRACE_H

#include "vm.h"

namespace jit {
// Interpreter modes:
// 1) Normal mode. Execute bytecode. If something become hot, goto 2. If encounter a compiled trace, run it
// 2) Recording mode. analyze the program, starting from the header after recording is done, switch to normal mode
    static int HOT_THRESHOLD = 1;
    static int RECORDING_TRACES = 20;

    enum class TraceResult {
        TRACE_FINISH,
        TRACE_ABORT,
        TRACE_TRY_AGAIN,
        TRACE_TRY_ANOTHER,
    };
    enum InstrType {
        ADDI,
        SUBI,
        MULI,
        DIVI,
        MODI,
        ADDF,
        SUBF,
        MULF,
        DIVF,
        MODF,
        ALLOC,
        ARRGET,
        ARRSET,
        CALL,
        JLE,
        JLS,
        JEQ,
        RET,
        RETNIL,
    };

    struct Trace {
        interpreter::VMData &vm;
    public:
        Trace(interpreter::VMData &vm);

        void parse_loadi(int a, int bx);

        void parse_mov(int a, int b);

    };

    struct TraceEntry {
        std::vector<jit::Trace> traces;

        void run(int idx);

        int try_entry(interpreter::VMData &vm);
    };
}


#endif //CRYPT_TRACE_H
