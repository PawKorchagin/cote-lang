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

using namespace testing;
using namespace parser;
using namespace ast;


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
    parser::init_parser(in);
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
inline void parse_program_throws(std::istream& fin, const std::string& file_name = "code") {
    parser::init_parser(fin);
    ast::Program p = parser::parse_program();
    if (!parser::get_errors().empty()) {
        for (auto x : get_errors()) {
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
