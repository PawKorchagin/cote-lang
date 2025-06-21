//
// Created by motya on 30.05.2025.
//

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
    template<typename T>
    inline __attribute__((always_inline)) void code_for_each(cfg::VMInfo &info, T func) {
        using namespace interpreter;
        for (int i = 0; i < info.code_size; ++i) {
//            const int opcode = info.code[i] >> interpreter::OPCODE_SHIFT;
//            uint8_t a = (info.code[i] >> A_SHIFT) & A_ARG;
//            uint8_t b = (info.code[i] >> B_SHIFT) & B_ARG;
//            uint8_t c = info.code[i] & C_ARG;
//            uint32_t bx = info.code[i] & BX_ARG;
            func(i, info.code[i]);
        }
    }

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

    template<typename A, typename I, typename... Args>
    void add_instruction(A &blocki, I &info, int mtype, Args &&... args) {
        blocki.emplace_back(mtype, info.size());
        info.emplace_back(std::forward<Args>(args)...);
    }

    void init_edges_to(cfg::CFGraph &graph, cfg::BasicBlock &block, int idx, std::unordered_map<int, int> &bjmp,
                       int id_count) {
        //block cannot be empty - it has at least first instruction with jump position
        //idx of the next block after the one we are processing
        const auto cur_instr = block.last->a;
        if (is_cjmp(cur_instr)) {
            const auto ajmp = ((int) cur_instr & (int) interpreter::BX_ARG) - (int) interpreter::J_ZERO + (int) idx;
            block.last->mtype = (cur_instr >> interpreter::OPCODE_SHIFT == interpreter::OP_JMPF
                                 ? cfg::InstrType::JMPF
                                 : cfg::InstrType::JMPT);
            block.last->a = bjmp.find(ajmp)->second;
            block.last->b = bjmp.find(idx)->second;
            return;
        }
        if (is_ret(cur_instr)) {
            block.last->mtype = cur_instr >> interpreter::OPCODE_SHIFT == interpreter::OP_RETURN ?
                                cfg::InstrType::RET : cfg::InstrType::RETNIL;

            block.last->a = id_count;
            block.last->b = ((int) cur_instr >> (int) interpreter::A_SHIFT) & (int) interpreter::A_ARG;
            return;
        }
        if (is_jmp(cur_instr)) {
            block.last->mtype = cfg::InstrType::JMP;
            const auto ajmp = ((int) cur_instr & (int) interpreter::BX_ARG) - (int) interpreter::J_ZERO + (int) idx;
            block.last->a = bjmp.find(ajmp)->second;
            return;
        }
        block.last->nxt = new cfg::BasicInstr(cfg::InstrType::JMP, bjmp.find(idx)->second);
        block.last = block.last->nxt;
    }

    void push_instr(cfg::BasicBlock &cur_block, int32_t instr) {
        using namespace interpreter;
//        switch (instr >> interpreter::OPCODE_SHIFT) {
//            case interpreter::OP_RETURN:
//                cur_block.last->nxt =  new cfg::BasicInstr(cfg::InstrType::RET, (instr >> (int)A_SHIFT) & (int)A_ARG);
//            case interpreter::OP_RETURNNIL:
//                cur_block.last->nxt =  new cfg::BasicInstr(cfg::InstrType::RETNIL);
//            default:
//        }
        cur_block.last->nxt = new cfg::BasicInstr(cfg::InstrType::PACKED, instr);
        cur_block.last = cur_block.last->nxt;
    }
}

bool cfg::CFGraph::buildBasicCFG() {
    if (info.code_size == 0) return false;
    //therefore we have at least two blocks: real first, fictitious(for now) last
    //jmp(to) position : block(to) id
    std::unordered_map<int, int> jmp_pos;
    //optimize case like this:
    //                  jmp here ->          instr1
    //                                       ...
    //                                       instr2
    //                                       jmp xxx
    //  but no jmp here(dead code) ->        instr3
    //                                       ...
    //makes like this: instr1, ..., instr2, jmp xxx
    // instr3 and etc. is removed
    // idea: put instr3 in should_jmp
    //TODO: implement this
    std::unordered_set<int> should_jmp;
    uint32_t id = 1;
    jmp_pos.emplace(0, 0);
    code_for_each(info,
                  [&id, &jmp_pos, &should_jmp](const int idx, const int instr) {
                      if (is_ret(instr)) {
                          should_jmp.insert(idx + 1);
                      }
                      if (!is_jmp(instr)) return;
                      const int bx = instr & (int) interpreter::BX_ARG;
                      const int pos = bx - (int32_t) interpreter::J_ZERO + idx + 1;
                      if (jmp_pos.emplace(pos, id).second) id++;
                      if (is_cjmp(instr)) {
                          if (jmp_pos.emplace(idx + 1, id).second) id++;
                      } else {
                          should_jmp.insert(idx + 1);
                      }
                  });
    blocks.resize(id + 2);
    for (auto &cur: should_jmp) {
        jmp_pos.emplace(cur, id + 1);
    }
    BasicBlock *cur_block = nullptr;
    for (int idx = 0; idx < info.code_size; ++idx) {
        const int instr = (int) info.code[idx];
        auto it = jmp_pos.find(idx);
        std::cerr << interpreter::ins_to_string(instr) << std::endl;
        if (it != jmp_pos.end()) {//block start
            const int temp_id = it->second;
            //process edge
            if (idx != 0 && cur_block->id != id + 1)
                init_edges_to(*this, *cur_block, idx, jmp_pos, (int) id);
//            init_edges_to(*this, *cur_block, idx, bjmp);
            cur_block = &blocks[temp_id];
            cur_block->id = temp_id;
            cur_block->ptr = idx;
            cur_block->first = cur_block->last = new BasicInstr(PACKED, instr);
        } else {
            push_instr(*cur_block, instr);
        }
    }
    init_edges_to(*this, *cur_block, info.code_size, jmp_pos, (int) id);
    blocks.back().finilize();
    blocks.pop_back();
    blocks[id].id = id;
    blocks[id].ptr = info.code_size;
    //clear last block TODO: clear jump_info(s)
    return !blocks.empty();
    //
}

std::ostream &cfg::CFGraph::toString(std::ostream &out) {
    out << "digraph {\n{node [shape=box fontsize=6]\n";
    for (auto &cur: blocks) {
        cur.write_name(out << "    ") << "\n";
    }
    out << "}\n";
    std::string names[3]{};
    for (int i = static_cast<int>(blocks.size()) - 2; i >= 0; --i) {
        const auto &cur = blocks[i];
        int cnt = 0;
        names[0] = std::to_string(reinterpret_cast<intptr_t>(&cur));
        switch (cur.last->mtype) {
            case static_cast<int>(InstrType::JMPT):
            case static_cast<int>(InstrType::JMPF):
                names[++cnt] = std::to_string(reinterpret_cast<intptr_t>(&blocks[cur.last->b]));
            case static_cast<int>(InstrType::JMP):
            case static_cast<int>(InstrType::RETNIL):
            case static_cast<int>(InstrType::RET):
                names[++cnt] = std::to_string(reinterpret_cast<intptr_t>(&blocks[cur.last->a]));
                break;
            default:
                throw std::runtime_error("block should not be empty");
        }
        out << names[0] << " -> { " << names[1];
        if (cnt == 2) {
            out << ", " << names[2] << " }\n";
        } else {
            out << "}\n";
        }
//        out << "{";
//        for (auto &prv: cur.from) {
//            out << std::to_string(reinterpret_cast<intptr_t>(&cur)) << " ";
//        }
//        out << "} -> " << std::to_string(reinterpret_cast<intptr_t>(&cur)) << "\n";
    }
    return out << "}";
}

std::ostream &cfg::BasicBlock::write_name(std::ostream &out) {
    out << std::to_string(reinterpret_cast<intptr_t>(this)) << " [label=<";
    out << "--- <b>b" << id << "</b> ---<br/>";
    for (auto it = this->first; it != nullptr; it = it->nxt) {
        out << cfg::ins_to_string(*it) << "<br/>";
    }
    return out << ">]";
}

std::string cfg::ins_to_string(const cfg::BasicInstr &instr) {
    if (instr.mtype == PACKED) return interpreter::ins_to_string(instr.a);
    switch (instr.mtype) {
        case static_cast<int>(InstrType::JMP):
            return "jmp b" + std::to_string(instr.a);
        case static_cast<int>(InstrType::JMPT):
            return "br b" + std::to_string(instr.a) + " b" + std::to_string(instr.b);
        case static_cast<int>(InstrType::JMPF):
            return "br b" + std::to_string(instr.b) + " b" + std::to_string(instr.a);
        case static_cast<int>(InstrType::RETNIL):
            return "ret nil";
        case static_cast<int>(InstrType::RET):
            return "ret b" + std::to_string(instr.b);
        default:
            throw std::runtime_error("unknown instr");
    }

}
