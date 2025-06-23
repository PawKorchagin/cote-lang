//
// Created by motya on 30.05.2025.
//

#ifndef COTE_NODES_H
#define COTE_NODES_H

#include <vector>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <unordered_map>
#include "asmjit/x86.h"
#include "vm.h"
#include "misc.h"
#include <cstring>
/*
Live statement:
1. stores into mem, performs I/O,
returns from function, calls function
that may have side effects
2. defines variable that is used
in a live statement
3. is a conditional branch that
affects whether a live statement is
executed (i.e., live statement is
control dependent on the branch)
 */
//bytecode limitations:
// - no stack (difficult to convert to IR) e.g. register-based bytecode
// - each function should end with ret nil
// - jmp limitations:
//      - no irreducible loops
namespace jit {
    enum class CompilationResult {
        ABORT,
        SUCCESS,
    };

    using FuncCompiled = uint64_t (*)(void *, void *);

    struct JITRuntime {

        CompilationResult compile(interpreter::VMData &vm, interpreter::Function &func, FuncCompiled &res);

    private:
        asmjit::JitRuntime asmrt;
    };

}

#endif //COTE_NODES_H
