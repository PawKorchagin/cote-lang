//
// Created by motya on 06.06.2025.
//
#include <format>
#include "bytecode_emitter.h"

void BytecodeEmitter::emit_add(int a, int b, int c) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_ADD, a, b, c));
}

void BytecodeEmitter::emit_sub(int a, int b, int c) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_SUB, a, b, c));

}

void BytecodeEmitter::emit_mul(int a, int b, int c) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_MUL, a, b, c));
}

void BytecodeEmitter::emit_div(int a, int b, int c) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_DIV, a, b, c));
}

void BytecodeEmitter::emit_mod(int a, int b, int c) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_MOD, a, b, c));
}

void BytecodeEmitter::emit_move(int a, int b) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_MOVE, a, b, 0));
}

void BytecodeEmitter::emit_neg(int a, int b) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_NEG, a, b, 0));
}

void BytecodeEmitter::emit_loadi(int a, int constant_offset) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_LOAD, a, constant_offset));
}

void BytecodeEmitter::emit_loadnil(int a) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_LOADNIL, a, 0, 0));
}

void BytecodeEmitter::emit_eq(int a, int b, int c) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_EQ, a, b, c));
}

void BytecodeEmitter::emit_less(int a, int b, int c) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_LT, a, b, c));
}

void BytecodeEmitter::emit_leq(int a, int b, int c) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_LE, a, b, c));
}


void BytecodeEmitter::emit_jtrue(int a, int offset) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_JMPT, a, offset));
}

void BytecodeEmitter::emit_jmp(int offset) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_JMP, 0, offset));
}

void BytecodeEmitter::emit_jfalse(int a, int offset) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_JMPF, a, offset));
}

void BytecodeEmitter::emit_return(int res) {
    using namespace interpreter;
    code.push_back(opcode(OpCode::OP_RETURN, res, 0, 0));
}

void BytecodeEmitter::begin_func(std::string name, int args) {
    std::cout << name << ": ; args=" << args << '\n';
}

void BytecodeEmitter::end_func() {
}
/*
                case OP_LOAD:
                    op_load(vm, a, bx);
                    break;
 */

void BytecodeEmitter::label(int32_t pos) {
    label_pos[pos] = static_cast<int>(code.size());
}

void BytecodeEmitter::resolve() {
    using namespace interpreter;
    for (auto &cur: pending_labels) {
        auto it = label_pos.find(cur.second);
        if (it == label_pos.end()) throw std::runtime_error("ill formed bytecode");
        auto& instr = code[cur.first];
        OpCode op      = static_cast<OpCode>(instr >> OPCODE_SHIFT);
        uint8_t a   = (instr >> A_SHIFT) & A_ARG;
        const int bx = (it->second - cur.first - 1);
        //TODO: SBX_MAX, ?LONG INSTRUCTIONS?
        if (abs(bx) > 10000) throw std::runtime_error("TODO: sbx is too large(code is too large), cannot emit jump");
        instr = opcode(op, a, bx);
    }
}

void BytecodeEmitter::jmp_label(int label) {
    using namespace interpreter;
    pending_labels.emplace_back(code.size(), label);
    code.push_back(opcode(OpCode::OP_JMP, 0, 0));
}

void BytecodeEmitter::jmpt_label(int a, int label) {
    using namespace interpreter;
    pending_labels.emplace_back(code.size(), label);
    code.push_back(opcode(OpCode::OP_JMPT, a, 0));
}

void BytecodeEmitter::jmpf_label(int a, int label) {
    using namespace interpreter;
    pending_labels.emplace_back(code.size(), label);
    code.push_back(opcode(OpCode::OP_JMPF, a, 0));
}





