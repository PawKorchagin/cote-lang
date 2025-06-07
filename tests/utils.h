//
// Created by Георгий on 14.04.2025.
//

#pragma once


#ifndef AUX_H
#define AUX_H

#include "gtest/gtest.h"
#include "lib/parser.h"
#include "lib/exceptions.h"
#include <random>
#include <utility>
#include <vector>
#include <stdexcept>
#include <unistd.h>
#include <ctime>
#include <string>
#include <fstream>
#include <format>

using namespace testing;
using namespace parser;
using namespace ast;

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
            return std::format("jmp {}", bx);
        }
        case OP_JMPT: {
            return std::format("jmpt [{}] {}", a, bx);
        }
        case OP_JMPF: {
            return std::format("jmpf [{}] {}", a, bx);
        }
        case OP_CALL:
            throw std::runtime_error("not supported");
            break;
        case OP_RETURN:
            return std::format("ret [{}]", a);
            break;
        case OP_NEWOBJ:
        case OP_GETFIELD:
        case OP_SETFIELD:
        case OP_HALT:
            throw std::runtime_error("todo");
            break;
        default:
            throw std::runtime_error("Unknown opcode");
    }
}

inline void print_func_body(std::vector<uint32_t> &code) {
    for (auto &cur: code) {
        std::cout << "    " << ins_to_string(cur) << "\n";
    }
}


/**
     * @brief Parses some code using given parser function.
     *
     *
     * @param text string containing code.
     * @param func specific function used to parse @a text. By default uses @a parse_expression
     * @throws std::runtime_error if result is a nullptr
     * @returns result of @a func
*/
template<typename T = decltype(parse_expression)>
inline auto parse(const std::string &text, T func = parse_expression) {
    auto in = std::stringstream(text);
    parser::init_parser(in, new BytecodeEmitter());
    auto expr = func();
    if (expr == nullptr) {
        std::cerr << text << " - " << get_errors().front() << std::endl;
        throw std::runtime_error("parser failed: " + get_errors().front());
    }
    return expr;
}

/**
     * @brief Parses a program, throws if errors occur.
     *
     * Program is read from @a fin
     *
     * @param fin input stream where the program code is located.
     * @param file_name file name if known. Affects only error messages
     * @throws std::runtime_error if parse errors occurs
*/
inline void parse_program_throws(std::istream &fin, const std::string &file_name = "code") {
    parser::init_parser(fin, new BytecodeEmitter());
    ast::Program p = parser::parse_program();
    if (!parser::get_errors().empty()) {
        for (auto x: get_errors()) {
            std::cerr << file_name << ":" << x << std::endl;
        }
        throw std::runtime_error("parser failed: " + get_errors().front());
    }
}

/**
     * @brief Parses program from file.
     *
     * Calls @a parse_program_throws on constructed input stream
*/
inline void parse_program_throws(std::string file_path) {
    std::ifstream fin(file_path);
    return parse_program_throws(fin, file_path);
}

/**
     * @brief Parses program from string.
     *
     * Calls @a parse_program_throws on constructed input stream
*/
inline void parse_throws_fromstr(std::string code) {
    std::stringstream ss(code);
    return parse_program_throws(ss, "<code>");
}

#endif //AUX_H
