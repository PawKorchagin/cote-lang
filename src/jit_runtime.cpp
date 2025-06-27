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
                                                jit::FuncCompiled &res) {
    using namespace interpreter;
    using namespace asmjit;
    CodeHolder holder;
    holder.init(asmrt.environment(), asmrt.cpuFeatures());
    jit::JitFuncInfo info(asmrt, holder, vm);
    if (vm.jit_log_level > 1) {
        FileLogger logger(stdout);
        holder.setLogger(&logger);
    }

    std::unordered_map<int, Label> labels;
    int start = func.entry_point;
    for (int i = 0; i < func.code_size; i++) {
        const uint32_t instr = vm.code[start++];
        const auto opcode = static_cast<OpCode>(instr >> OPCODE_SHIFT);
        if (opcode == OP_JMP || opcode == OP_JMPF || opcode == OP_JMPT) {
            const int32_t sbx = static_cast<int32_t>(instr & BX_ARG) - J_ZERO;
            const int loc = i + 1 + sbx;
            auto it = labels.find(loc);
            if (it == labels.end()) {
                labels.emplace(loc, info.cc.newLabel());
            }
        }

    }
    start -= func.code_size;

    //                                                          Value (*) (Value* vm)
    FuncNode *node = info.cc.addFunc(FuncSignature::build<uint64_t, void *>());

    info.arg1 = info.cc.newUIntPtr("args*");       // Create `dst` register (destination pointer).

    node->setArg(0, info.arg1);


    for (int i = 0; i < func.code_size; i++) {

//TODO: keep info about register, if it is a merge flow point, reset it. Use it to optimize type information. Basically equivalent to local basic block optimizations.
//        auto it = merge_flow_points.find(i);
//        if (it == merge_flow_points.end()) {
//            info.clear();
//        }
        auto it = labels.find(i);
        if (it != labels.end()) {
            info.cc.bind(it->second);
        }

        const uint32_t instr = vm.code[start++];


        const uint8_t a = (instr >> A_SHIFT) & A_ARG;
        const uint8_t b = (instr >> B_SHIFT) & B_ARG;
        const uint8_t c = instr & C_ARG;
        const uint32_t bx = instr & BX_ARG;
        const int32_t sbx = static_cast<int32_t>(instr & BX_ARG) - J_ZERO;
        const int jump_loc = i + 1 + sbx;
//        std::cerr << ins_to_string(instr) << std::endl;

        switch (static_cast<OpCode>(instr >> OPCODE_SHIFT)) {
            case interpreter::OP_ADD:
                info.binary_operation<OP_ADD>(a, b, c);
                break;
            case interpreter::OP_SUB:
                info.binary_operation<OP_SUB>(a, b, c);
                break;
            case interpreter::OP_MUL:
                info.binary_operation<OP_MUL>(a, b, c);
                break;
            case interpreter::OP_DIV:
                info.binary_operation<OP_DIV>(a, b, c);
                break;
            case OP_LT: {
                info.binary_operation<OP_LT>(a, b, c);
                break;
            }
            case OP_LE: {
                info.binary_operation<OP_LE>(a, b, c);
                break;
            }
            case interpreter::OP_LOADINT: {
                auto temp = info.cc.newUInt64();
                info.cc.movabs(temp, vm.constanti[bx].as_uint64());
                info.cc.mov(x86::ptr(info.arg1, a * 8), temp);
                break;
            }
            case interpreter::OP_JMPF: {
                info.cjmp<false>(a, labels[1 + i + sbx]);
                break;
            }
            case interpreter::OP_NATIVE_CALL: {
                info.native_call((void *) vm.natives[a], b, c);
                break;
            }
            case interpreter::OP_CALL:
                throw std::runtime_error("cannot compile");
                break;
            case interpreter::OP_RETURN: {
                auto temp = info.cc.newUInt64();
                info.cc.mov(temp, x86::dword_ptr(info.arg1, a * 8));
                info.cc.mov(x86::dword_ptr(info.arg1), temp);
                info.cc.ret(temp);
            }
                break;
            case interpreter::OP_RETURNNIL: {
                auto v = info.cc.newUInt64();
                info.cc.movabs(v, OBJ_NIL);
                info.cc.mov(x86::dword_ptr(info.arg1), v);
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
                info.cc.mov(x86::dword_ptr(info.arg1, a * 8), temp);
                break;
            }
            case OP_MOD: {
                info.modulo_operation(a, b, c);
                break;
            }
            case OP_NEG: {
                info.neg(a, b);
            }
                break;
            case OP_EQ: {
                auto t1 = info.cc.newUInt64();
                auto t2 = info.cc.newUInt64();
                info.cc.mov(t1, x86::dword_ptr(info.arg1, b * 8));
                info.cc.mov(t2, x86::dword_ptr(info.arg1, c * 8));
                info.cc.bts(t1, 33);
                info.cc.bts(t2, 33);
                info.cc.cmp(t1, t2);
                info.cc.sete(x86::word_ptr(info.arg1, a * 8));
                info.cc.mov(x86::word_ptr(info.arg1, a * 8 + 4), TYPE_INT);
            }
                break;
            case OP_NEQ: {
                auto t1 = info.cc.newUInt64();
                auto t2 = info.cc.newUInt64();
                info.cc.mov(t1, x86::dword_ptr(info.arg1, b * 8));
                info.cc.mov(t2, x86::dword_ptr(info.arg1, c * 8));
                info.cc.bts(t1, 33);
                info.cc.bts(t2, 33);
                info.cc.cmp(t1, t2);
                info.cc.setne(x86::word_ptr(info.arg1, a * 8));
                info.cc.mov(x86::word_ptr(info.arg1, a * 8 + 4), TYPE_INT);
            }
                break;
            case OP_JMP: {
                info.cc.jmp(labels[i + 1 + sbx]);
                break;
            }
            case OP_JMPT: {
                info.cjmp<true>(a, labels[1 + i + sbx]);
                break;
            }
            case OP_INVOKEDYNAMIC:
                break;
            case OP_HALT:
                throw std::runtime_error("cannot compile");
                break;
            case OP_LOADFUNC: {
                throw std::runtime_error("not supported");
//                Value v;
//                v.set_callable(bx);
//                auto temp = info.cc.newUInt64();
//                info.cc.movabs(temp, v.as_uint64());
//                info.cc.movabs(x86::ptr(info.arg1, a * 8), temp);
//                break;
            }
            case OP_LOADFLOAT: {
                auto temp = info.cc.newUInt64();
                info.cc.movabs(temp, vm.constantf[bx].as_uint64());
                info.cc.mov(x86::ptr(info.arg1, a * 8), temp);
                break;
            }
            case OP_ALLOC: {
                throw std::runtime_error("not supported");
                break;
            }
            case OP_ARRGET: {
                info.op_arrget(instr);
                break;
            }
            case OP_ARRSET: {
                info.op_arrset(instr);
                break;
            }
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

jit::FuncCompiled jit::JitRuntime::compile_safe(interpreter::VMData &vm, interpreter::Function &func) {
    try {
        FuncCompiled res;
        if (compile(vm, func, res) != CompilationResult::SUCCESS) return nullptr;
        return res;
    } catch (...) {
        return nullptr;
    }
}


void jit::JitFuncInfo::modulo_operation(int a, int b, int c) {
    using namespace interpreter;
    using namespace asmjit;
    auto err = cc.newLabel();
    auto nxt = cc.newLabel();

    auto temp2 = cc.newUInt64();
    auto temp3 = cc.newUInt64();

    {//int * int
        cc.cmp(x86::word_ptr(arg1, b * 8 + 4), TYPE_INT);
        cc.jne(err);
        cc.cmp(x86::word_ptr(arg1, c * 8 + 4), TYPE_INT);
        cc.jne(err);
        auto temp = cc.newInt32();
        cc.mov(temp, x86::word_ptr(arg1, b * 8));
        interpreter::Value tempInt;
        tempInt.set_int(0);
        {
            cc.cmp(x86::word_ptr(arg1, c * 8), 0);
            cc.je(err);
            x86::Gp dummy2 = cc.newInt32();
            cc.xor_(dummy2, dummy2);
            cc.idiv(dummy2, temp, x86::word_ptr(arg1, c * 8));
            cc.mov(temp, dummy2);
        }
        cc.mov(x86::word_ptr(arg1, a * 8), temp);
        cc.mov(x86::word_ptr(arg1, a * 8 + 4), TYPE_INT);
        cc.mov(temp2, x86::dword_ptr(arg1, a * 8));
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

void jit::JitFuncInfo::neg(int a, int b) {
    using namespace asmjit;
    using namespace interpreter;
    auto err = cc.newLabel();
    auto sf = cc.newLabel();
    auto nxt = cc.newLabel();
    {
        cc.cmp(x86::word_ptr(arg1, b * 8 + 4), TYPE_INT);
        cc.jne(sf);
        auto temp = cc.newInt32();
        cc.mov(temp, x86::word_ptr(arg1, b * 8));
        cc.neg(temp);
        cc.mov(x86::word_ptr(arg1, a * 8), temp);
        cc.mov(x86::word_ptr(arg1, a * 8 + 4), TYPE_INT);
        cc.jmp(nxt);
    }
    {
        cc.bind(sf);
        cc.cmp(x86::word_ptr(arg1, b * 8 + 4), TYPE_FLOAT);
        cc.jne(err);
        auto xmm = cc.newXmmSs();
        cc.xorps(xmm, xmm);
        cc.subss(xmm, x86::word_ptr(arg1, b * 8));
        cc.movd(x86::dword_ptr(arg1, a * 8), xmm);
        cc.mov(x86::word_ptr(arg1, a * 8 + 4), TYPE_FLOAT);
        cc.jmp(nxt);
    }
    cc.bind(err);
    auto failCode = cc.newUInt64();
    cc.movabs(failCode, OBJ_NIL);
    cc.add(failCode, 1);
    cc.ret(failCode);
    cc.bind(nxt);
}

namespace {

    asmjit::x86::Gp getArg1() {
#if defined(_WIN32)
        return asmjit::x86::rcx;
#else
        return asmjit::x86::rdi;
#endif
    }

    asmjit::x86::Gp getArg2() {
#if defined(_WIN32)
        return asmjit::x86::rdx;
#else
        return asmjit::x86::rsi;
#endif
    }

    asmjit::x86::Gp getArg3() {
#if defined(_WIN32)
        return asmjit::x86::r8;
#else
        return asmjit::x86::rdx;
#endif
    }
}

void jit::JitFuncInfo::native_call(void *func, int b, int c) {
    using namespace asmjit;
    cc.push(getArg1());
    cc.push(getArg2());
    cc.push(getArg3());
    cc.push(x86::rax);//save registers
    cc.sub(x86::rsp, 32);//move rsp
    cc.mov(getArg1(), &vm);//set up args...
    cc.mov(getArg2(), b);
    cc.mov(getArg3(), c);
    cc.mov(x86::rax, func);
    cc.call(x86::rax);//call
    cc.add(x86::rsp, 32);//mov back rsp
    cc.pop(x86::rax);//restore registers
    cc.pop(getArg3());
    cc.pop(getArg2());
    cc.pop(getArg1());
}

void jit::JitFuncInfo::op_arrget(int instr) {

}

void jit::JitFuncInfo::op_arrset(int instr) {

}


