//
// Created by motya on 06.06.2025.
//
#include <format>
#include "bytecode_emitter.h"
#include <cstring>

void interpreter::BytecodeEmitter::emit_add(int a, int b, int c) {
    using namespace interpreter;
    add(opcode(OpCode::OP_ADD, a, b, c));
}

void interpreter::BytecodeEmitter::emit_sub(int a, int b, int c) {
    using namespace interpreter;
    add(opcode(OpCode::OP_SUB, a, b, c));

}

void interpreter::BytecodeEmitter::emit_mul(int a, int b, int c) {
    using namespace interpreter;
    add(opcode(OpCode::OP_MUL, a, b, c));
}

void interpreter::BytecodeEmitter::emit_div(int a, int b, int c) {
    using namespace interpreter;
    add(opcode(OpCode::OP_DIV, a, b, c));
}

void interpreter::BytecodeEmitter::emit_mod(int a, int b, int c) {
    using namespace interpreter;
    add(opcode(OpCode::OP_MOD, a, b, c));
}

void interpreter::BytecodeEmitter::emit_move(int a, int b) {
    using namespace interpreter;
    add(opcode(OpCode::OP_MOVE, a, b, 0));
}

void interpreter::BytecodeEmitter::emit_neg(int a, int b) {
    using namespace interpreter;
    add(opcode(OpCode::OP_NEG, a, b, 0));
}

// Value creation helpers
namespace {
    interpreter::Value int_val(int32_t v) {
        using namespace interpreter;
        Value val;
        val.set_int(v);
        return val;
    }
}

void interpreter::BytecodeEmitter::emit_loadi(int a, int imm) {
    auto &it = iconstants[imm];
    if (it == 0) it = ++iconstant_count;
    add(opcode(OpCode::OP_LOADINT, a, it - 1));
}

void interpreter::BytecodeEmitter::emit_loadnil(int a) {
    using namespace interpreter;
    add(opcode(OpCode::OP_LOADNIL, a, 0, 0));
}

void interpreter::BytecodeEmitter::emit_eq(int a, int b, int c) {
    using namespace interpreter;
    add(opcode(OpCode::OP_EQ, a, b, c));
}

void interpreter::BytecodeEmitter::emit_less(int a, int b, int c) {
    using namespace interpreter;
    add(opcode(OpCode::OP_LT, a, b, c));
}

void interpreter::BytecodeEmitter::emit_leq(int a, int b, int c) {
    using namespace interpreter;
    add(opcode(OpCode::OP_LE, a, b, c));
}


void interpreter::BytecodeEmitter::emit_jtrue(int a, int offset) {
    using namespace interpreter;
    add(opcode(OpCode::OP_JMPT, a, offset));
}

void interpreter::BytecodeEmitter::emit_jmp(int offset) {
    using namespace interpreter;
    add(opcode(OpCode::OP_JMP, 0, offset));
}

void interpreter::BytecodeEmitter::emit_jfalse(int a, int offset) {
    using namespace interpreter;
    add(opcode(OpCode::OP_JMPF, a, offset));
}

void interpreter::BytecodeEmitter::emit_return(int res) {
    using namespace interpreter;
    add(opcode(OpCode::OP_RETURN, res, 0, 0));
}

//in the end of each function resolve labels;
//the code is like this:
//fn definitions with thier innner labels
//...
//global code with its labels
//fn definitions
//...
//therefore:
// - in the end of each function we resolve
// - and in begin_func and in the absolute end resolve.
int interpreter::BytecodeEmitter::begin_func(int args, std::string name) {
    resolve();
    funcs[cur_func].name = std::move(name);
    funcs[cur_func].arity = args;
    funcs[cur_func].code.clear();
    is_in_func = true;
    return cur_func;
}

void interpreter::BytecodeEmitter::end_func() {
    resolve();
    is_in_func = false;
    cur_func++;
}

/*
                case OP_LOAD:
                    op_load(vm, a, bx);
                    break;
 */

void interpreter::BytecodeEmitter::label(int32_t pos) {
    label_pos[pos] = static_cast<int>(get().size());
}

void interpreter::BytecodeEmitter::resolve() {
    using namespace interpreter;
    for (auto &cur: pending_labels) {
        auto it = label_pos.find(cur.second);
        if (it == label_pos.end()) throw std::runtime_error("ill formed bytecode");
        auto &instr = get()[cur.first];
        const auto op = static_cast<OpCode>(instr >> OPCODE_SHIFT);
        uint8_t a = (instr >> A_SHIFT) & A_ARG;
        const int bx = (it->second - cur.first - 1);
        //TODO: SBX_MAX, ?LONG INSTRUCTIONS?
        if (abs(bx) > 10000) throw std::runtime_error("TODO: sbx is too large(code is too large), cannot emit jump");
        instr = opcode(op, a, bx + (int32_t) J_ZERO);
    }
    pending_labels.clear();
    label_pos.clear();
}

void interpreter::BytecodeEmitter::jmp_label(int label) {
    using namespace interpreter;
    pending_labels.emplace_back(get().size(), label);
    add(opcode(OpCode::OP_JMP, 0, label));
}

void interpreter::BytecodeEmitter::jmpt_label(int a, int label) {
    using namespace interpreter;
    pending_labels.emplace_back(get().size(), label);
    add(opcode(OpCode::OP_JMPT, a, 0));
}

void interpreter::BytecodeEmitter::jmpf_label(int a, int label) {
    using namespace interpreter;
    pending_labels.emplace_back(get().size(), label);
    add(opcode(OpCode::OP_JMPF, a, 0));
}


void interpreter::BytecodeEmitter::initVM(interpreter::VMData &vm) {
    resolve();
    int cur = -1;
    for (int i = 0; i < cur_func; ++i) {
        if (funcs[i].name == "main")
            cur = i;
    }
    if (cur == -1) throw std::runtime_error("no main method found");
    emit_call_direct(cur, 0, 0);
    emit_halt();
    vm.constanti.resize(iconstant_count);
    for (auto &it: iconstants) {
        vm.constanti[it.second - 1].set_int(it.first);
    }
    vm.constantf.resize(fconstant_count);
    for (auto &it: fconstants) {
        vm.constantf[it.second - 1].set_float(it.first);
    }
    size_t offset = 0;
    vm.functions_count = cur_func;
    for (int i = 0; i < cur_func; ++i) {
        vm.functions[i].arity = funcs[i].arity;
        vm.functions[i].entry_point = offset;
        vm.functions[i].code_size = funcs[i].code.size();
        std::memcpy(vm.code + offset, funcs[i].code.data(), funcs[i].code.size() * sizeof(uint32_t));
        offset += funcs[i].code.size();
    }
    vm.ip = offset;
    vm.sp = vm.fp = 0;
    std::memcpy(vm.code + offset, global.data(), global.size() * sizeof(uint32_t));
    vm.code_size = global.size() + offset;
}

void interpreter::BytecodeEmitter::emit_halt() {
    add(opcode(OP_HALT, 0, 0));
}

void interpreter::BytecodeEmitter::emit_retnil() {
    add(opcode(OP_RETURNNIL, 0, 0));
}

void interpreter::BytecodeEmitter::emit_call(int funcid, int reg, int count) {
    add(opcode(OpCode::OP_INVOKEDYNAMIC, funcid, reg, count));
}

void interpreter::BytecodeEmitter::emit_loadfunc(uint32_t reg, uint32_t fid) {
    add(opcode(OpCode::OP_LOADFUNC, reg, fid));
}

void interpreter::BytecodeEmitter::emit_arrayget(int to, int from, int offset) {
    add(opcode(OpCode::OP_ARRGET, to, from, offset));
}

void interpreter::BytecodeEmitter::emit_arrayset(int to, int offset, int from) {
    add(opcode(OpCode::OP_ARRSET, to, offset, from));
}


void interpreter::BytecodeEmitter::emit_native(int id, int from, int cnt) {
    add(opcode(OpCode::OP_NATIVE_CALL, id, from, cnt));
}

void interpreter::BytecodeEmitter::emit_call_direct(int funcid, int reg, int count) {
    add(opcode(OpCode::OP_CALL, funcid, reg, count));
}

void interpreter::BytecodeEmitter::emit_alloc(uint32_t reg, uint32_t reg2) {
    add(opcode(OpCode::OP_ALLOC, reg, reg2, 0));
}

void interpreter::BytecodeEmitter::emit_neq(int a, int b, int c) {
    add(opcode(OpCode::OP_NEQ, a, b, c));
}

void interpreter::BytecodeEmitter::emit_loadf(uint32_t reg, float imm) {
    auto &it = fconstants[imm];
    if (it == 0) it = ++fconstant_count;
    add(opcode(OpCode::OP_LOADFLOAT, reg, it - 1));
}





