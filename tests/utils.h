//
// Created by Георгий on 14.04.2025.
//

#pragma once


#ifndef AUX_H
#define AUX_H

#include "gtest/gtest.h"
#include "src/parser.h"
#include "src/exceptions.h"
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

#include "src/ins_to_string.h"

inline void print_func_body(uint32_t *code, int msize) {
    for (int i = 0; i < msize; ++i) {
        std::cout << "    " << interpreter::ins_to_string(code[i]) << "\n";
    }
}

inline void print_vm_data(interpreter::VMData &vm) {
    using namespace interpreter;
    std::cout << "constants: [";
    for (int i = 0; i < vm.constanti.size(); ++i) {
        std::cout << vm.constanti[i].i32;
        if (i != vm.constanti.size() - 1) std::cout << ", ";
    }
    std::cout << "]\n";
    std::unordered_map<int, Function *> functions;
    for (int i = 0; i < vm.functions_count; ++i) {
        functions[vm.functions[i].entry_point] = &vm.functions[i];
    }
    for (int i = 0; i < vm.code_size; ++i) {
        auto it = functions.find(i);
        if (it != functions.end()) {
            std::cout << "func" << it->second - vm.functions << "(args: " << (int) it->second->arity << "):\n";
        }
        std::cout << "    " << interpreter::ins_to_string(vm.code[i], &vm.constanti, &vm.constantf);
        if (i == vm.ip) {
            std::cout << " <- ip";
        }
        std::cout << '\n';
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
    parser::init_parser(in, new interpreter::BytecodeEmitter());
    auto expr = func();
    if (expr == nullptr) {
        std::cerr << text << " - " << get_errors().front() << std::endl;
        throw std::runtime_error("parser failed: " + get_errors().front());
    }
    return expr;
}

inline interpreter::VMData &initVM() {
    interpreter::VMData &vm = interpreter::vm_instance();
    // vm.gc = heap::GarbageCollector();

    vm.ip = 0; // Start at first instruction
    vm.fp = 0; // Frame pointer at base
    vm.sp = 0; // Stack pointer

    return vm;
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
    parser::init_parser(fin, new interpreter::BytecodeEmitter());
    parser::parse_program(interpreter::vm_instance());
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
