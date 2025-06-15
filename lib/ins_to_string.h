//
// Created by motya on 13.06.2025.
//

#ifndef CRYPT_INS_TO_STRING_H
#define CRYPT_INS_TO_STRING_H

#include "vm.h"
#include <format>

namespace interpreter {

//-- for debugging --
    inline std::string ins_to_string(uint32_t instr) {
        using namespace interpreter;

        OpCode op = static_cast<OpCode>(instr >> OPCODE_SHIFT);

        uint8_t a = (instr >> A_SHIFT) & A_ARG;
        uint8_t b = (instr >> B_SHIFT) & B_ARG;
        uint8_t c = instr & C_ARG;
        uint32_t bx = instr & BX_ARG;
        switch (op) {
            case OP_LOAD:
                return std::format("mov [{}] {}", a, bx);
            case OP_MOVE:
                return std::format("mov [{}] [{}]", a, b);
            case OP_LOADNIL:
                return std::format("mov [{}] nil", a);
            case OP_ADD:
                return std::format("add [{}] [{}] [{}]", a, b, c);
            case OP_SUB:
                return std::format("sub [{}] [{}] [{}]", a, b, c);
            case OP_MUL:
                return std::format("mul [{}] [{}] [{}]", a, b, c);
            case OP_DIV:
                return std::format("div [{}] [{}] [{}]", a, b, c);
            case OP_MOD:
                return std::format("mod [{}] [{}] [{}]", a, b, c);
            case OP_NEG:
                return std::format("neg [{}] [{}] [{}]", a, b, c);
            case OP_EQ:
                return std::format("eq [{}] [{}] [{}]", a, b, c);
            case OP_LT:
                return std::format("lt [{}] [{}] [{}]", a, b, c);
            case OP_LE:
                return std::format("leq [{}] [{}] [{}]", a, b, c);
            case OP_JMP: {
                return std::format("jmp {}", (int32_t) bx - (int32_t) J_ZERO);
            }
            case OP_JMPT: {
                return std::format("jmpt [{}] {}", a, (int32_t) bx - (int32_t) J_ZERO);
            }
            case OP_JMPF: {
                return std::format("jmpf [{}] {}", a, (int32_t) bx - (int32_t) J_ZERO);
            }
            case OP_CALL:
                return std::format("call f{} [{}]...[{}]", a, b, c);
            case OP_RETURN:
                return std::format("ret [{}]", a);
                break;
            case OP_NEWOBJ:
            case OP_GETFIELD:
            case OP_SETFIELD:
            case OP_HALT:
                return "halt";
            case OP_RETURNNIL:
                return std::format("ret nil");
            default:
                throw std::runtime_error("Unknown opcode");
        }
    }
}

#endif //CRYPT_INS_TO_STRING_H
