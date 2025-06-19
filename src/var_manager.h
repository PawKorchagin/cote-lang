//
// Created by motya on 06.06.2025.
//

#ifndef COTE_VARIABLE_MANAGER_H
#define COTE_VARIABLE_MANAGER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "bytecode_emitter.h"

namespace parser {
    struct LoopManager {
        int label_start;
        int label_end;

        LoopManager(int labelStart = 0, int labelEnd = 0);
    };

    struct LevelInfo {
        int sp = 0;
        std::vector<std::string> vars;

        LevelInfo(int sp = 0) : sp(sp), vars{} {}
    };

    struct VarManager {
        typedef void(*NativeFunction)(interpreter::VMData &);

        inline VarManager() : levels(1, LevelInfo()), var_names() {}

        inline int last() { return levels.back().sp - 1; }

        inline int push_var() { return levels.back().sp++; }

        inline int pop_var() { return --levels.back().sp; }

        inline void drop(int n) { levels.back().sp -= n; }

        int get_var(std::string name);

        int push_var(const std::string &name);

        void new_scope();

        void close_scope();

        int get_func(std::string name);

        bool add_func(std::string name, int fid);

        int get_native(std::string name);

        int add_native(std::string name);

    private:
        std::vector<LevelInfo> levels;
        std::unordered_map<std::string, int> functions;
        std::unordered_map<std::string, int> natives;
        // "x" : <location, level>
        std::unordered_map<std::string, std::pair<int, int>> var_names;
    };
}


#endif //COTE_VARIABLE_MANAGER_H
