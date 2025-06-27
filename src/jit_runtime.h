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

    using FuncCompiled = uint64_t (*)(void *);

    static constexpr uint64_t HIGH32 = 18446744069414584320ull;
    static constexpr uint64_t ERR_TYPE = interpreter::OBJ_NIL + 1;

    struct JitRuntime {


        CompilationResult
        compile(interpreter::VMData &vm, interpreter::Function &func, FuncCompiled &res);

        FuncCompiled compile_safe(interpreter::VMData &vm, interpreter::Function &func);

    private:
        asmjit::JitRuntime asmrt;
    };

    struct JitFuncInfo {
        asmjit::JitRuntime &asmrt;
        interpreter::VMData &vm;
        asmjit::CodeHolder &holder;
        asmjit::x86::Compiler cc;
        asmjit::x86::Gp arg1, arg2;

        inline JitFuncInfo(asmjit::JitRuntime &jit, asmjit::CodeHolder &holder, interpreter::VMData &vm) : asmrt(jit),
                                                                                                           holder(holder),
                                                                                                           cc(&this->holder),
                                                                                                           vm(vm) {}

        template<int mtype>
        void
        binary_operation(int a, int b, int c);

        void modulo_operation(int a, int b, int c);

        void native_call(void *func, int b, int c);

        inline void op_arrget(int instr) { throw std::runtime_error("not supported"); }

        inline void op_arrset(int instr) { throw std::runtime_error("not supported"); }

        template<bool jmpT>
        void cjmp(int a, const asmjit::Label &label) {
            using namespace asmjit;
            using namespace interpreter;
            auto sf = cc.newLabel();
            auto obj = cc.newLabel();
            auto nxt = cc.newLabel();
            {
                cc.cmp(x86::word_ptr(arg1, a * 8 + 4), TYPE_INT);
                cc.jne(sf);
                cc.cmp(x86::word_ptr(arg1, a * 8), 0);
                if constexpr (jmpT) {
                    cc.jne(label);
                } else {
                    cc.je(label);
                }
                cc.jmp(nxt);
            }
            {
                cc.bind(sf);
                const float cnst = 0.0f;
                cc.cmp(x86::word_ptr(arg1, a * 8 + 4), TYPE_FLOAT);
                cc.jne(obj);
                cc.cmp(x86::word_ptr(arg1, a * 8), *reinterpret_cast<const int *>(&cnst));
                if constexpr (jmpT) {
                    cc.jne(label);
                } else {
                    cc.je(label);
                }
                cc.jmp(nxt);
            }
            {
                cc.bind(obj);
                cc.cmp(x86::word_ptr(arg1, a * 8 + 4), TYPE_OBJ);
                if constexpr (jmpT) {
                    cc.jne(label);
                } else {
                    cc.je(label);
                }
                cc.jmp(nxt);
            }
            cc.bind(nxt);
        }

        void neg(int a, int b);
    };

    template<int mtype>
    void
    jit::JitFuncInfo::binary_operation(int a, int b, int c) {
        using namespace interpreter;
        using namespace asmjit;
        auto err = cc.newLabel();
        auto sf = cc.newLabel();
        auto nxt = cc.newLabel();

        auto temp2 = cc.newUInt64();
        auto temp3 = cc.newUInt64();

        {//int * int
            cc.cmp(x86::word_ptr(arg1, b * 8 + 4), TYPE_INT);
            cc.jne(sf);
            cc.cmp(x86::word_ptr(arg1, c * 8 + 4), TYPE_INT);
            cc.jne(err);//sif
            auto temp = cc.newInt32();
            cc.mov(temp, x86::ptr(arg1, b * 8));
            interpreter::Value tempInt;
            tempInt.set_int(0);
            if constexpr (mtype == interpreter::OP_ADD) {
                cc.add(temp, x86::word_ptr(arg1, c * 8));
            } else if constexpr (mtype == interpreter::OP_SUB) {
                cc.sub(temp, x86::word_ptr(arg1, c * 8));
            } else if constexpr (mtype == interpreter::OP_MUL) {
                cc.imul(temp, x86::word_ptr(arg1, c * 8));
            } else if constexpr (mtype == interpreter::OP_DIV) {
                cc.cmp(x86::word_ptr(arg1, c * 8), 0);
                cc.je(err);
                x86::Gp dummy2 = cc.newInt32();
                cc.xor_(dummy2, dummy2);
                cc.idiv(dummy2, temp, x86::word_ptr(arg1, c * 8));
            } else if constexpr (mtype == interpreter::OP_LT) {
                cc.cmp(temp, x86::word_ptr(arg1, c * 8));
                cc.setl(x86::al);
                cc.mov(x86::word_ptr(arg1, a * 8), x86::al);
                cc.mov(x86::word_ptr(arg1, a * 8 + 4), TYPE_INT);
            } else if constexpr (mtype == interpreter::OP_LE) {
                cc.cmp(temp, x86::word_ptr(arg1, c * 8));
                cc.setle(x86::al);
                cc.mov(x86::word_ptr(arg1, a * 8), x86::al);
                cc.mov(x86::word_ptr(arg1, a * 8 + 4), TYPE_INT);
            }
            if constexpr (mtype != interpreter::OP_LT && mtype != interpreter::OP_LE) {
                cc.mov(x86::word_ptr(arg1, a * 8), temp);
                cc.mov(x86::word_ptr(arg1, a * 8 + 4), TYPE_INT);
            }
            cc.jmp(nxt);
        }
        {
            interpreter::Value tempInt;
            tempInt.set_float(0);
            //TODO: float * float
            cc.bind(sf);
            cc.cmp(x86::word_ptr(arg1, b * 8 + 4), TYPE_FLOAT);
            cc.jne(err);
            cc.cmp(x86::word_ptr(arg1, c * 8 + 4), TYPE_FLOAT);
            cc.jne(err);//sfi
            auto temp = cc.newXmmSs();
            cc.movss(temp, x86::word_ptr(arg1, b * 8));
            if constexpr (mtype == interpreter::OP_ADD) {
                cc.addss(temp, x86::dword_ptr(arg1, c * 8));
            } else if constexpr (mtype == interpreter::OP_SUB) {
                cc.subss(temp, x86::dword_ptr(arg1, c * 8));
            } else if constexpr (mtype == interpreter::OP_MUL) {
                cc.mulss(temp, x86::dword_ptr(arg1, c * 8));
            } else if constexpr (mtype == interpreter::OP_DIV) {
                cc.divss(temp, x86::dword_ptr(arg1, c * 8));
            } else if constexpr (mtype == interpreter::OP_LT) {
                cc.comiss(temp, x86::dword_ptr(arg1, c * 8));
                cc.setl(x86::al);
                cc.mov(x86::word_ptr(arg1, a * 8), x86::al);
                cc.mov(x86::word_ptr(arg1, a * 8 + 4), TYPE_INT);
            } else if constexpr (mtype == interpreter::OP_LE) {
                cc.comiss(temp, x86::dword_ptr(arg1, c * 8));
                cc.setle(x86::al);
                cc.mov(x86::word_ptr(arg1, a * 8), x86::al);
                cc.mov(x86::word_ptr(arg1, a * 8 + 4), TYPE_INT);
            }
            if constexpr (mtype != interpreter::OP_LT && mtype != interpreter::OP_LE) {
                cc.movd(x86::dword_ptr(arg1, a * 8), temp);
                cc.mov(x86::word_ptr(arg1, a * 8 + 4), TYPE_FLOAT);
            }
            cc.jmp(nxt);
        }


        cc.bind(err);
        auto failCode = cc.newUInt64();
        cc.movabs(failCode, OBJ_NIL);
        cc.add(failCode, 1);
        cc.ret(failCode);
        cc.bind(nxt);

//    cc.movabs(temp2, 18446744069414584320ull);
//    cc.and_(stack[a], temp2);
    }
}

#endif //COTE_NODES_H
