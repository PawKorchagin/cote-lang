//
// Created by motya on 21.06.2025.
//

#include "JITRuntime.h"

#include <algorithm>
#include <queue>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <iostream>
#include <cassert>
#include "ins_to_string.h"

namespace {

}

jit::CompilationResult jit::JITRuntime::compile(interpreter::VMData &vm,
                                                interpreter::Function &func,
                                                jit::FuncCompiled &res) {
    using namespace interpreter;
    using namespace asmjit;
    CodeHolder holder;
    holder.init(asmrt.environment(), asmrt.cpuFeatures());
    x86::Compiler compiler(&holder);
    //                                                          Value (*) (Value* vm, Function* func)
    FuncNode *node = compiler.addFunc(FuncSignature::build<uint64_t, void *, void *>());

    x86::Gp vmarg = compiler.newUIntPtr("args*");       // Create `dst` register (destination pointer).
    x86::Gp farg = compiler.newUIntPtr("func*");       // Create `dst` register (destination pointer).
    x86::Mem stack = compiler.newStack(256, 4);
    x86::Gp sp = compiler.newUInt64("i");
//    working example:
//    x86::Gp op1 = compiler.newInt32();
//    x86::Gp ret1 = compiler.newInt64();
//    x86::Gp ret2 = compiler.newInt64();
//
//    node->setArg(0, vmarg);
//    node->setArg(1, farg);
//    // -- code --
//    compiler.mov(ret1, x86::dword_ptr(vmarg, 0));
//    compiler.mov(x86::dword_ptr(sp), ret1);
//
//    compiler.mov(ret1, x86::dword_ptr(vmarg, 8));
//    compiler.mov(x86::dword_ptr(sp, 8), ret1);
//
//    compiler.mov(op1, x86::word_ptr(sp));
//    compiler.add(op1, x86::word_ptr(sp, 8));
//    compiler.movsx(ret2, op1);//sum in ret2
//
//
//    compiler.mov(ret1, ((uint64_t) TYPE_INT << 32ull));
//    compiler.or_(ret1, ret2);
//    compiler.ret(ret1);
    // Runtime specialized for JIT code execution.
    FileLogger logger(stdout);
    holder.setLogger(&logger);
//    so if should_jump contains something, it's because it's dead code
    for (int i = 0; i < func.code_size; i++) {

//TODO: keep info about register, if it is a merge flow point, reset it. Use it to optimize type information. Basically equivalent to local basic block optimizations.
//        auto it = merge_flow_points.find(i);
//        if (it == merge_flow_points.end()) {
//            info.clear();
//        }


        const uint32_t instr = vm.code[vm.ip++];


        const uint8_t a = (instr >> A_SHIFT) & A_ARG;
        const uint8_t b = (instr >> A_SHIFT) & A_ARG;
        const uint32_t bx = instr & BX_ARG;
        const uint8_t c = instr & C_ARG;

        std::cerr << ins_to_string(instr) << std::endl;

        switch (static_cast<OpCode>(instr >> OPCODE_SHIFT)) {
            case interpreter::OP_ADD: {
                auto op1 = compiler.newInt32();
                auto ret1 = compiler.newInt64();
                compiler.mov(op1, x86::word_ptr(sp, (b - func.arity) * 8));
                compiler.add(op1, x86::word_ptr(sp, (c - func.arity) * 8));
                compiler.movsx(ret1, op1);//sum in ret2


                compiler.mov(x86::dword_ptr(sp, (a - func.arity) * 8), ((uint64_t) TYPE_INT << 32ull));
                compiler.or_(x86::dword_ptr(sp, (a - func.arity) * 8), ret1);
                break;
            }
            case interpreter::OP_LOADINT: {
                compiler.mov(x86::dword_ptr(sp, (a - func.arity) * 8),
                             *reinterpret_cast<uint64_t *>(&vm.constanti[bx]));
                break;
            }
            case interpreter::OP_JMPF:
                break;
            case interpreter::OP_RETURN: {
                auto reg = compiler.newUInt64();
                compiler.mov(reg, x86::dword_ptr(sp, (a - func.arity) * 8));
                compiler.ret(reg);
                break;
            }
            case interpreter::OP_RETURNNIL:
                break;
        }

    }
    compiler.endFunc();
    compiler.finalize();
    asmjit::Error err = asmrt.add(&res, &holder);          // Add the generated code to the runtime.
    if (err)
        return jit::CompilationResult::ABORT;
    return jit::CompilationResult::SUCCESS;
}


