//
// Created by Георгий on 15.04.2025.
//

#ifndef EXCEPTION_H
#define EXCEPTION_H
#include <stdexcept>

class lvalue_error final : std::runtime_error {
public:
    explicit lvalue_error(const std::string &basic_string)
        : runtime_error(basic_string) {
    }

    explicit lvalue_error(const char *string)
        : runtime_error(string) {
    }
};

#endif //EXCEPTION_H
