//
// Created by motya on 21.06.2025.
//

#include "jit_runtime.h"

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

jit::CompilationResult jit::JitRuntime::compile(interpreter::VMData &vm,
                                                interpreter::Function &func,
                                                jit::FuncCompiled &res, int osr_size) {
    using namespace interpreter;
    using namespace asmjit;
    CodeHolder holder;
    holder.init(asmrt.environment(), asmrt.cpuFeatures());
    jit::JitFuncInfo info(asmrt, holder);

    FileLogger logger(stdout);
    holder.setLogger(&logger);

    //                                                          Value (*) (int osr_size, Value* vm, Function* func)
    FuncNode *node = info.cc.addFunc(FuncSignature::build<uint64_t, void *, void *>());

    info.arg1 = info.cc.newUIntPtr("args*");       // Create `dst` register (destination pointer).
    info.arg2 = info.cc.newUIntPtr("func*");       // Create `dst` register (destination pointer).
    x86::Gp jtarget = info.cc.newInt32();

    node->setArg(0, info.arg1);
    node->setArg(1, info.arg2);
//    node->setArg(2, jtarget);

//    info.cc.jmp(jtarget);



    for (int i = 0; i < func.code_size; i++) {

//TODO: keep info about register, if it is a merge flow point, reset it. Use it to optimize type information. Basically equivalent to local basic block optimizations.
//        auto it = merge_flow_points.find(i);
//        if (it == merge_flow_points.end()) {
//            info.clear();
//        }


        const uint32_t instr = vm.code[vm.ip++];


        const uint8_t a = (instr >> A_SHIFT) & A_ARG;
        const uint8_t b = (instr >> B_SHIFT) & B_ARG;
        const uint8_t c = instr & C_ARG;
        const uint32_t bx = instr & BX_ARG;


//        std::cerr << ins_to_string(instr) << std::endl;

        switch (static_cast<OpCode>(instr >> OPCODE_SHIFT)) {
            case interpreter::OP_ADD:
                info.binary_operation<OP_ADD>(a, b, c);
                break;
            case interpreter::OP_SUB:
                info.binary_operation<OP_ADD>(a, b, c);
                break;
            case interpreter::OP_LOADINT: {
                auto temp = info.cc.newUInt64();
                info.cc.movabs(temp, vm.constanti[bx].as_uint64());
                info.cc.mov(x86::ptr(info.arg1, a * 8), temp);
                break;
            }
            case interpreter::OP_JMPF:
                break;
            case interpreter::OP_NATIVE_CALL:
                break;
            case interpreter::OP_CALL:
                break;
            case interpreter::OP_RETURN: {
                auto temp = info.cc.newUInt64();
                info.cc.mov(temp, x86::ptr(info.arg1, a * 8));
                info.cc.ret(temp);
            }
                break;
            case interpreter::OP_RETURNNIL: {
                auto v = info.cc.newUInt64();
                info.cc.movabs(v, OBJ_NIL);
                info.cc.ret(v);
            }
                break;
            case OP_MOVE: {
                auto temp = info.cc.newUInt64();
                info.cc.mov(temp, x86::ptr(info.arg1, b * 8));
                info.cc.mov(x86::ptr(info.arg1, a * 8), temp);
            }
                break;
            case OP_LOADNIL: {
                Value v;
                v.set_nil();
                auto temp = info.cc.newUInt64();
                info.cc.movabs(temp, v.as_uint64());
                info.cc.mov(x86::ptr(info.arg1, a * 8), v.as_uint64());
                break;
            }
            case OP_MUL:
                break;
            case OP_DIV:
                break;
            case OP_MOD:
                break;
            case OP_NEG:
                break;
            case OP_EQ:
                break;
            case OP_NEQ:
                break;
            case OP_LT:
                break;
            case OP_LE:
                break;
            case OP_JMP:
                break;
            case OP_JMPT:
                break;
            case OP_INVOKEDYNAMIC:
                break;
            case OP_HALT:
                throw std::runtime_error("cannot compile");
                break;
            case OP_LOADFUNC: {
                Value v;
                v.set_callable(bx);
                auto temp = info.cc.newUInt64();
                info.cc.movabs(temp, v.as_uint64());
                info.cc.movabs(x86::ptr(info.arg1, a * 8), temp);
                break;
            }
            case OP_LOADFLOAT: {
                auto temp = info.cc.newUInt64();
                info.cc.movabs(temp, vm.constantf[bx].as_uint64());
                info.cc.mov(x86::ptr(info.arg1, a * 8), temp);
                break;
            }
            case OP_ALLOC: {

                break;
            }
            case OP_ARRGET:
                break;
            case OP_ARRSET:
                break;
            default:
                throw std::runtime_error("not supported");
        }

    }
    info.cc.endFunc();
    info.cc.finalize();
    asmjit::Error err = asmrt.add(&res, &holder);          // Add the generated code to the runtime.
    if (err)
        return jit::CompilationResult::ABORT;
    return jit::CompilationResult::SUCCESS;
}


void jit::JitFuncInfo::bailout() {
    for (int i = 0; i < 256; ++i) {
//        cc.mov(asmjit::x86::dword_ptr(arg1, i * 8), stack[i]);
    }
    cc.ret();
}


