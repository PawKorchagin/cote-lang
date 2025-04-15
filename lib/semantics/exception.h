//
// Created by Георгий on 15.04.2025.
//

#ifndef EXCEPTION_H
#define EXCEPTION_H
#include <stdexcept>

// class lvalue_error final : std::runtime_error {
// public:
//     explicit lvalue_error(const std::string &basic_string)
//         : runtime_error(basic_string) {
//     }
//
//     explicit lvalue_error(const char *string)
//         : runtime_error(string) {
//     }
// };
//
// class rvalue_error final : std::runtime_error {
// public:
//     explicit rvalue_error(const std::string &basic_string)
//         : runtime_error(basic_string) {
//     }
//
//     explicit rvalue_error(const char *string)
//         : runtime_error(string) {
//     }
// };

using lvalue_error = std::runtime_error;
using rvalue_error = std::runtime_error;

#endif //EXCEPTION_H
