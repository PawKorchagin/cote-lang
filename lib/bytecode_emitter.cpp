//
// Created by motya on 06.06.2025.
//
#include <format>
#include "bytecode_emitter.h"
#include <cstring>

void interpreter::BytecodeEmitter::emit_add(int a, int b, int c) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_ADD, a, b, c));
}

void interpreter::BytecodeEmitter::emit_sub(int a, int b, int c) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_SUB, a, b, c));

}

void interpreter::BytecodeEmitter::emit_mul(int a, int b, int c) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_MUL, a, b, c));
}

void interpreter::BytecodeEmitter::emit_div(int a, int b, int c) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_DIV, a, b, c));
}

void interpreter::BytecodeEmitter::emit_mod(int a, int b, int c) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_MOD, a, b, c));
}

void interpreter::BytecodeEmitter::emit_move(int a, int b) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_MOVE, a, b, 0));
}

void interpreter::BytecodeEmitter::emit_neg(int a, int b) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_NEG, a, b, 0));
}

// Value creation helpers
namespace {
    interpreter::Value int_val(int32_t v) {
        interpreter::Value val;
        val.type = interpreter::ValueType::Int;
        val.as.i32 = v;
        return val;
    }
}

void interpreter::BytecodeEmitter::emit_loadi(int a, int imm) {
    auto &it = iconstants[imm];
    if (it == 0) it = ++iconstant_count;
    code.push_back(opcode(OpCode::OP_LOAD, a, it - 1));
}

void interpreter::BytecodeEmitter::emit_loadnil(int a) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_LOADNIL, a, 0, 0));
}

void interpreter::BytecodeEmitter::emit_eq(int a, int b, int c) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_EQ, a, b, c));
}

void interpreter::BytecodeEmitter::emit_less(int a, int b, int c) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_LT, a, b, c));
}

void interpreter::BytecodeEmitter::emit_leq(int a, int b, int c) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_LE, a, b, c));
}


void interpreter::BytecodeEmitter::emit_jtrue(int a, int offset) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_JMPT, a, offset));
}

void interpreter::BytecodeEmitter::emit_jmp(int offset) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_JMP, 0, offset));
}

void interpreter::BytecodeEmitter::emit_jfalse(int a, int offset) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_JMPF, a, offset));
}

void interpreter::BytecodeEmitter::emit_return(int res) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_RETURN, res, 0, 0));
}

void interpreter::BytecodeEmitter::begin_func(std::string name, int args) {
    std::cout << name << ": ; args=" << args << '\n';
}

void interpreter::BytecodeEmitter::end_func() {
}

/*
                case OP_LOAD:
                    op_load(vm, a, bx);
                    break;
 */

void interpreter::BytecodeEmitter::label(int32_t pos) {
    label_pos[pos] = static_cast<int>(code.size());
}

void interpreter::BytecodeEmitter::resolve() {
    using namespace interpreter;
    for (auto &cur: pending_labels) {
        auto it = label_pos.find(cur.second);
        if (it == label_pos.end()) throw std::runtime_error("ill formed bytecode");
        auto &instr = code[cur.first];
        const auto op = static_cast<OpCode>(instr >> OPCODE_SHIFT);
        uint8_t a = (instr >> A_SHIFT) & A_ARG;
        const int bx = (it->second - cur.first - 1);
        //TODO: SBX_MAX, ?LONG INSTRUCTIONS?
        if (abs(bx) > 10000) throw std::runtime_error("TODO: sbx is too large(code is too large), cannot emit jump");
        instr = opcode(op, a, bx + (int32_t) J_ZERO);
    }
}

void interpreter::BytecodeEmitter::jmp_label(int label) {
    using namespace interpreter;
    pending_labels.emplace_back(code.size(), label);
    code.push_back(opcode(OpCode::OP_JMP, 0, label));
}

void interpreter::BytecodeEmitter::jmpt_label(int a, int label) {
    using namespace interpreter;
    pending_labels.emplace_back(code.size(), label);
    code.push_back(opcode(OpCode::OP_JMPT, a, 0));
}

void interpreter::BytecodeEmitter::jmpf_label(int a, int label) {
    using namespace interpreter;
    pending_labels.emplace_back(code.size(), label);
    code.push_back(opcode(OpCode::OP_JMPF, a, 0));
}


void interpreter::BytecodeEmitter::initVM(interpreter::VMData &vm) {
    vm.constants.resize(iconstant_count);
    for (auto& it : iconstants) {
        vm.constants[it.second - 1].as.i32 = it.first;
        vm.constants[it.second - 1].type = ValueType::Int;
    }
    std::memcpy(vm.code, code.data(), code.size() * sizeof(uint32_t));
    vm.code_size = code.size();
}

void interpreter::BytecodeEmitter::emit_halt() {
    code.push_back(opcode(OP_HALT, 0, 0));
}

void interpreter::BytecodeEmitter::emit_retnil() {
    code.push_back(opcode(OP_RETURNNIL, 0, 0));
}

void interpreter::BytecodeEmitter::emit_call(int funcid, int reg, int count) {
    code.push_back(opcode(OpCode::OP_INVOKEDYNAMIC, funcid, reg, count));
}





