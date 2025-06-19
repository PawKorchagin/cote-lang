//
// Created by motya on 06.06.2025.
//

#ifndef COTE_BYTECODE_EMITTER_H
#define COTE_BYTECODE_EMITTER_H

#include "misc.h"
#include "vm.h"
#include <iostream>
#include <unordered_map>

namespace interpreter {
    struct EmitFunc {
        std::string name;
        std::vector<uint32_t> code;
        int arity = 0;
    };

    struct BytecodeEmitter {
        std::unordered_map<int32_t, int32_t> label_pos;
        std::vector<std::pair<int, int>> pending_labels;
        std::vector<uint32_t> global;
        std::unordered_map<int, int> iconstants;
        int iconstant_count = 0;
        int cur_func = 0;
        bool is_in_func = false;
        EmitFunc funcs[1024];

        //TODO: use unordered_map


    public:
        void emit_loadfunc(uint32_t reg, uint32_t fid);

        void label(int32_t pos);

        void resolve();

        //returns this func id
        int begin_func(int args, std::string name);

        void end_func();

        void emit_halt();

        void emit_arrayget(int to, int from, int offset);

        void emit_arrayset(int to, int offset, int from);

        void emit_native(int id, int from, int cnt);

        void emit_retnil();

        void emit_call(int funcid, int reg, int count);

        // Adds two values
        // Args: a - destination, b - first operand, c - second operand
        // Behavior: registers[a] = registers[b] + registers[c] (int/float)
        void emit_add(int a, int b, int c);

        // Subtracts two values
        // Args: Same as OP_ADD
        // Behavior: registers[a] = registers[b] - registers[c]
        void emit_sub(int a, int b, int c);

        // Multiplies two values
        // Args: Same as OP_ADD
        // Behavior: registers[a] = registers[b] * registers[c]
        void emit_mul(int a, int b, int c);


        // Divides two values
        // Args: Same as OP_ADD
        // Behavior: registers[a] = registers[b] / registers[c]
        void emit_div(int a, int b, int c);

        // Division remainder of two values(integer only)
        // Args: Same as OP_ADD
        // Behavior: registers[a] = registers[b] % registers[c]
        void emit_mod(int a, int b, int c);

        // Copies value between registers
        // Args: a - destination register, b - source register
        // Behavior: registers[a] = registers[b]
        void emit_move(int a, int b);

        // Arithmetic negation
        // Args: a - destination, b - source register
        // Behavior: registers[a] = -registers[b]
        void emit_neg(int a, int b);

        // Loads a constant into a register
        // Args: a - destination register, bx - constant pool index
        // Behavior: registers[a] = constants[offset](offset - index of bx in a pool of constants, added automaticly)
        void emit_loadi(int a, int val);

        // Loads nil into a register
        // Args: a - target register
        // Behavior: registers[a] = nil
        void emit_loadnil(int reg);

        // Equality comparison
        // Args: a - result register (1/0), b - first operand, c - second operand
        // Behavior: registers[a] = (registers[b] == registers[c]) ? 1 : 0
        void emit_eq(int a, int b, int c);

        // Less-than comparison
        // Args: Same as OP_EQ
        // Behavior: registers[a] = (registers[b] < registers[c]) ? 1 : 0
        void emit_less(int a, int b, int c);

        // Less-or-equal comparison
        // Args: Same as OP_EQ
        // Behavior: registers[a] = (registers[b] <= registers[c]) ? 1 : 0
        void emit_leq(int a, int b, int c);

        // Unconditional jump
        // Args: sbx - signed offset
        // Behavior: ip += offset
        void emit_jmp(int offset);

        void jmp_label(int label);

        void jmpt_label(int a, int label);

        void jmpf_label(int a, int label);

        // Jump if true
        // Args: a - condition register, sbx - signed offset
        // Behavior: if (registers[a]) ip += offset
        void emit_jtrue(int a, int offset);

        // Jump if false
        // Args: Same as OP_JMPT
        // Behavior: if (!registers[a]) ip += offset
        void emit_jfalse(int a, int offset);

        // Function call
        // Args: a - function index, b - argument count
        // Behavior:
        //   1. Pushes current ip/fp to call stack
        //   2. Sets new fp = sp
        //   3. sp += local_count
        //   4. Jumps to function entry_point
        //TODO: void emit_call(....);


        // Return from function
        // Args: a - result register
        // Behavior:
        //   1. Restores ip/fp from call stack
        //   2. Sets sp = fp
        //   3. Stores result in caller's register 0
        void emit_return(int res);

        // Creates new object
        // Args: a - destination register, bx - class index
        // Behavior:
        //   1. Allocates heap memory
        //   2. Initializes fields
        //   3. registers[a] = object reference
        //TODO: OP_NEWOBJ,

        // Reads object field
        // Args: a - destination, b - object register, c - field index
        // Behavior: registers[a] = object(b).fields[c]
        //TODO: OP_GETFIELD,

        // Writes object field
        // Args: a - object register, b - field index, c - value register
        // Behavior: object(a).fields[b] = registers[c]
        //TODO: OP_SETFIELD,

        // Stops execution
        // Args: none
        // Behavior: terminates VM execution
        //TODO: OP_HALT

        void initVM(interpreter::VMData &vm);

        inline uint32_t pop() {
            uint32_t res;
            if (is_in_func) {
                res = funcs[cur_func].code.back();
                funcs[cur_func].code.pop_back();
            } else {
                res = global.back();
                global.pop_back();
            }
            return res;
        }

    private:

        inline void add(uint32_t instr) {
            if (is_in_func) {
                funcs[cur_func].code.push_back(instr);
            } else {
                global.push_back(instr);
            }
        }

        inline std::vector<uint32_t> &get() {
            if (is_in_func) return funcs[cur_func].code;
            else return global;
        }

    };

}

#endif //COTE_BYTECODE_EMITTER_H
