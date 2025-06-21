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

    enum TraceResult {
        TRACE_FINISH,
        TRACE_ABORT,
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
    class Trace {
        void eval_code(interpreter::VMData& data, int head);
        void record_add(interpreter::VMData& data);
    };
}


#endif //CRYPT_TRACE_H
