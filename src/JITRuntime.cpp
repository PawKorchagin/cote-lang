//
// Created by motya on 21.06.2025.
//

#include "JITRuntime.h"

#include <algorithm>
#include <queue>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <iostream>
#include <cassert>
#include "nodes.h"
#include "ins_to_string.h"

namespace {
    int is_jmp(int instr) {
        const int opcode = instr >> interpreter::OPCODE_SHIFT;
        return opcode == interpreter::OP_JMP || opcode == interpreter::OP_JMPF || opcode == interpreter::OP_JMPT;
    }

    int is_ret(int instr) {
        const int opcode = instr >> interpreter::OPCODE_SHIFT;
        return opcode == interpreter::OP_RETURNNIL || opcode == interpreter::OP_RETURN;
    }

    int is_cjmp(int instr) {
        const int opcode = instr >> interpreter::OPCODE_SHIFT;
        return opcode == interpreter::OP_JMPF || opcode == interpreter::OP_JMPT;
    }
}

void cfg::JITRuntime::compile(interpreter::VMData &vm, interpreter::Function &func) {
    //so if should_jump contains something, it's because it's dead code
    for (int i = 0; i < func.code_size; i++) {
        //TODO: keep info about register, if it is a merge flow point, reset it. Use it to optimize type information. Basically equivalent to local basic block optimizations.
//        auto it = merge_flow_points.find(i);
//        if (it == merge_flow_points.end()) {
//            info.clear();
//        }
        switch (vm.code[i + vm.ip]) {
            case interpreter::OP_ADD:

                break;
            case interpreter::OP_JMPF:
                break;
            case interpreter::OP_RETURN:
                break;
            case interpreter::OP_RETURNNIL:
                break;
        }
    }
}

