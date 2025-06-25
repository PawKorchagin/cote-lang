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

    static constexpr uint64_t HIGH32 = 18446744069414584320ull;
    static constexpr uint64_t ERR_TYPE = interpreter::OBJ_NIL + 1;

    struct JitRuntime {


        CompilationResult
        compile(interpreter::VMData &vm, interpreter::Function &func, FuncCompiled &res, int osr);


    private:
        asmjit::JitRuntime asmrt;
    };

    struct JitFuncInfo {
        asmjit::JitRuntime &asmrt;
        asmjit::CodeHolder &holder;
        asmjit::x86::Compiler cc;
        asmjit::x86::Gp arg1, arg2;
        asmjit::x86::Gp stack[256];

        inline JitFuncInfo(asmjit::JitRuntime &jit, asmjit::CodeHolder &holder) : asmrt(jit), holder(holder),
                                                                                  cc(&this->holder) {}

        template<int mtype>
        void
        binary_operation(int a, int b, int c);

        inline void bailout() { throw std::runtime_error("todo"); }

        void emit_osr(int osr_size);
    };

    template<int mtype>
    void
    jit::JitFuncInfo::binary_operation(int a, int b, int c) {
        using namespace interpreter;
        using namespace asmjit;
        auto err = cc.newLabel();
        auto sif = cc.newLabel();
        auto sf = cc.newLabel();
        auto sfi = cc.newLabel();
        auto nxt = cc.newLabel();

        auto temp2 = cc.newUInt64();
        auto temp3 = cc.newUInt64();
        cc.mov(temp2, stack[b]);
        cc.shr(temp2, 32);
        cc.mov(temp3, stack[c]);
        cc.shr(temp3, 32);
        {//int * int
            cc.cmp(temp2, TYPE_INT);
            cc.jne(sf);
            cc.cmp(temp3, TYPE_INT);
            cc.jne(err);//sif

            auto temp = cc.newInt32();
            cc.mov(temp, stack[b].r32());
            interpreter::Value tempInt;
            tempInt.set_int(0);
            if constexpr (mtype == interpreter::OP_ADD) {
                cc.add(temp, stack[c].r32());
            } else if constexpr (mtype == interpreter::OP_SUB) {
                cc.sub(temp, stack[c].r32());
            } else if constexpr (mtype == interpreter::OP_MUL) {
                cc.mul(temp, stack[c].r32());
            } else if constexpr (mtype == interpreter::OP_DIV) {
                cc.div(temp, stack[c].r32());
            }

            //extend with zeros
            cc.movzx(stack[a], temp);
            cc.movabs(temp2, tempInt.as_uint64());
            cc.or_(stack[a], temp2);
            cc.jmp(nxt);//int * int end
        }
        {
            interpreter::Value tempInt;
            tempInt.set_float(0);
            //TODO: float * float
            cc.bind(sf);
            cc.cmp(temp2, TYPE_FLOAT);
            cc.jne(err);
            cc.cmp(temp3, TYPE_FLOAT);
            cc.jne(err);//sfi
            cc.mov(x86::dword_ptr(arg1), stack[b].r32());
            cc.mov(x86::dword_ptr(arg1, 8), stack[c].r32());
            auto temp = cc.newXmmSs();
            cc.movss(temp, x86::dword_ptr(arg1));
            if constexpr (mtype == interpreter::OP_ADD) {
                cc.addss(temp, x86::dword_ptr(arg1, 8));
            } else if constexpr (mtype == interpreter::OP_SUB) {
                cc.subss(temp, x86::dword_ptr(arg1, 8));
            } else if constexpr (mtype == interpreter::OP_MUL) {
                cc.mulss(temp, x86::dword_ptr(arg1, 8));
            } else if constexpr (mtype == interpreter::OP_DIV) {
                cc.mulss(temp, x86::dword_ptr(arg1, 8));
            }
            cc.movd(stack[a].r32(), temp);
            cc.movabs(temp2, tempInt.as_uint64());
            cc.or_(stack[a], temp2);
            cc.jmp(nxt);
        }


        cc.bind(err);
        auto failCode = cc.newUInt64();
        cc.movabs(failCode, OBJ_NIL);
        cc.add(failCode, 1);
        cc.ret(failCode);
        cc.bind(nxt);
//    cc.bind(err);
//    cc.ret();

//    cc.movabs(temp2, 18446744069414584320ull);
//    cc.and_(stack[a], temp2);
    }
}

#endif //COTE_NODES_H
