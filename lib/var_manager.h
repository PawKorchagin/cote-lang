//
// Created by motya on 06.06.2025.
//

#ifndef CRYPT_VARIABLE_MANAGER_H
#define CRYPT_VARIABLE_MANAGER_H

#include <string>
#include <vector>
#include <unordered_map>
#include "bytecode_emitter.h"

namespace parser {
    struct VarManager {
        std::unordered_map<std::string, int> var_names;
        std::vector<std::vector<std::string>> locals;
        std::vector<int> sp;

        inline VarManager() : sp({0}), locals(1, std::vector<std::string>{}), var_names() {}

        inline int last() { return sp.back() - 1; }

        inline int push_var() { return sp.back()++; }

        inline int pop_var() { return --sp.back(); }

        int get_var(std::string name);

        int push_var(const std::string &name);


        void new_scope();

        void close_scope();
    };
}


#endif //CRYPT_VARIABLE_MANAGER_H
