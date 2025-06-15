//
// Created by motya on 30.05.2025.
//

#ifndef CRYPT_NODES_H
#define CRYPT_NODES_H

#include <vector>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <unordered_map>
#include "vm.h"
#include "misc.h"
#include <cstring>
/*
Live statement:
1. stores into mem, performs I/O,
returns from function, calls function
that may have side effects
2. defines variable that is used
in a live statement
3. is a conditional branch that
affects whether a live statement is
executed (i.e., live statement is
control dependent on the branch)
 */
//bytecode limitations:
// - no stack (difficult to convert to IR) e.g. register-based bytecode
// - each function should end with ret nil
// - jmp limitations:
//      - no irreducible loops
namespace cfg {

    enum InstrType {
        JMP,
        JMPF,
        JMPT,
        RET,
        RETNIL,
        PACKED,
        NONE,
    };

    struct BasicInstr {
        uint32_t mtype;
        int32_t a;
        int32_t b;
        int32_t c;
        BasicInstr *nxt;

        explicit BasicInstr(int mtype = NONE, int32_t a = 0, int32_t b = 0, int32_t c = 0, BasicInstr *nxt = nullptr) :
                mtype(mtype), a(a), b(b), c(c), nxt(nxt) {}
    };

    struct BasicLoop;

    /*
     * Block with no control flow splits/merges
     * - starts with either:
     *      - first instruction in bytecode
     *      - has jump to this location(either explicit(jmp) or not as explicit(jmpt, jmpf))
     * - ends with either:
     *      - last instruction in bytecode
     *      - jump to another block
     */
    struct BasicBlock {
        BasicInstr *first = nullptr;
        BasicInstr *last = nullptr;
        uint32_t id = 0;
        uint32_t ptr = 0;
        std::vector<BasicBlock *> from;
        BasicLoop *loop;

        BasicBlock(uint32_t id = 0) : id(id) {}

        std::ostream &write_name(std::ostream &out);

        inline void finilize() const {
            for (auto it = first; it != nullptr;) {
                const auto cur = it;
                it = it->nxt;
                delete cur;
            }
        }
    };

    std::string ins_to_string(const BasicInstr& instr);
    struct BasicLoop {

    };

    struct VMInfo {
        uint32_t *code = nullptr;
        int code_size = 0;

        VMInfo(interpreter::VMData &data) : code(data.code), code_size(data.code_size) {}
        //statistics etc.
    };

    //invariants:
    // - Last block doesn't have a jump and any return points to the last block
    // - Last block contains only one instructions(return )//for now it contains 0
    struct CFGraph {
        std::vector<BasicBlock> blocks;
        VMInfo info;

        inline CFGraph(const VMInfo &info) : info(std::move(info)) {}

        bool buildBasicCFG();

        std::ostream &toString(std::ostream &out);
    };


}

#endif //CRYPT_NODES_H
