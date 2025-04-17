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
#include <ctime>
#include <string>
#include <fstream>

using namespace testing;
using namespace parser;
using namespace ast;

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

inline void parse_program_throws(std::string file_path) {
    std::ifstream fin(file_path);
    parser::init_parser(fin);
    ast::Program p = parser::parse_program();
    if (!parser::get_errors().empty()) {
        for (auto x : get_errors()) {
            std::cerr << file_path << ":" << x << std::endl;
        }
        throw std::runtime_error("parser failed: " + get_errors().front());
    }
}


#endif //AUX_H
