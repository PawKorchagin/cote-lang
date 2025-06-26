#ifndef CRYPT_VM_H
#define CRYPT_VM_H

#include <cstdint>
#include <fstream>
#include <stack>
#include <unordered_map>
#include <vector>
#include <memory>

#include "heap.h"
#include "misc.h"
#include "value.h"

namespace jit {
    struct Trace;
    struct TraceEntry;
    enum class TraceResult;
}

namespace interpreter {
    // static auto gc = heap::GarbageCollector();

    // template<uint16_t GC_YOUNG_THRESHOLD=50>
    struct VMData;

    enum OpCode {
        // Loads a constant into a register
        // Args: a - destination register, bx - constant pool index
        // Behavior: registers[a] = constants[bx]
        OP_LOADINT,

        // Copies value between registers
        // Args: a - destination register, b - source register
        // Behavior: registers[a] = registers[b]
        OP_MOVE,

        // Loads nil into a register
        // Args: a - target register
        // Behavior: registers[a] = nil
        OP_LOADNIL,

        // Adds two values
        // Args: a - destination, b - first operand, c - second operand
        // Behavior: registers[a] = registers[b] + registers[c] (int/float)
        OP_ADD,

        // Subtracts two values
        // Args: Same as OP_ADD
        // Behavior: registers[a] = registers[b] - registers[c]
        OP_SUB,

        // Multiplies two values
        // Args: Same as OP_ADD
        // Behavior: registers[a] = registers[b] * registers[c]
        OP_MUL,

        // Divides two values
        // Args: Same as OP_ADD
        // Behavior: registers[a] = registers[b] / registers[c]
        OP_DIV,

        // Modulo operation (integers only)
        // Args: Same as OP_ADD
        // Behavior: registers[a] = registers[b] % registers[c]
        OP_MOD,

        // Arithmetic negation
        // Args: a - destination, b - source register
        // Behavior: registers[a] = -registers[b]
        OP_NEG,

        // Equality comparison
        // Args: a - result register, b - first operand, c - second operand
        // Behavior: registers[a] = (registers[b] == registers[c]) ? 1 : 0
        OP_EQ,//TODO

        // Args: a - result register, b - first operand, c - second operand
        // Behavior: registers[a] = (registers[b] != registers[c]) ? 1 : 0
        OP_NEQ,//TODO

        // Less-than comparison
        // Args: Same as OP_EQ
        // Behavior: registers[a] = (registers[b] < registers[c]) ? 1 : 0
        OP_LT,

        // Less-or-equal comparison
        // Args: Same as OP_EQ
        // Behavior: registers[a] = (registers[b] <= registers[c]) ? 1 : 0
        OP_LE,

        // Unconditional jump
        // Args: sbx - signed offset
        // Behavior: ip += sbx
        OP_JMP,

        // Jump if true
        // Args: a - condition register, sbx - signed offset
        // Behavior: if (registers[a]) ip += sbx
        OP_JMPT,//TODO

        // Jump if false
        // Args: Same as OP_JMPT
        // Behavior: if (!registers[a]) ip += sbx
        OP_JMPF,//TODO

        // Function call
        // Args:
        // a - function index,
        // b - index of register with first argument,
        // c - argument count
        // Behavior:
        //   1. Pushes current ip/fp to call stack
        //   2. Sets new fp = sp
        //   3. sp += local_count
        //   4. Jumps to function entry_point
        OP_CALL,

        // Calls native function from `natives`
        // Args:
        // a - native function index
        // Behavior:
        // natives[a](vm)
        OP_NATIVE_CALL,//TODO

        // Call function dynamically
        // Args:
        // a - register index with callable value
        // b - index of register with first argument,
        // c - argument count
        // Behavior:
        // call functions[register[a]] b c
        OP_INVOKEDYNAMIC,

        // Return from function
        // Args: a - result register
        // Behavior:
        //   1. Restores ip/fp from call stack
        //   2. Sets sp = fp
        //   3. Stores result in caller's register 0
        OP_RETURN,

        // Return nil from function
        // Behavior:
        // Works like return, but writes NIL to R0
        OP_RETURNNIL,

        // Stops execution
        // Args: none
        // Behavior: terminates VM execution
        OP_HALT,

        OP_LOADFUNC,//TODO
        OP_LOADFLOAT,
        OP_ALLOC,//TODO
        OP_ARRGET,//TODO
        OP_ARRSET,//TODO
        OP_TAILCALL,
    };
    //TODO: parse float in parser


    typedef void (*NativeFunction)(VMData &, int reg, int cnt);

    struct ObjClass {
        std::pmr::unordered_map<std::string, int> indexes;
    };

// Memory limits
    static constexpr uint32_t CODE_MAX_SIZE = 4096;
    static constexpr uint32_t STACK_SIZE = 4096;
    static constexpr uint32_t HEAP_MAX_SIZE = 65536;
    static constexpr uint32_t FUNCTIONS_MAX = 256;
    static constexpr uint32_t CALL_MAX_SIZE = 2000;

// Dispatch constants
    static constexpr uint32_t A_ARG = 0xFF;
    static constexpr uint32_t A_SHIFT = 18;
    static constexpr uint32_t B_ARG = 0x1FF;
    static constexpr uint32_t B_SHIFT = 9;
    static constexpr uint32_t C_ARG = 0x1FF;
    static constexpr uint32_t BX_ARG = 0x3FFFF;
    static constexpr uint32_t OPCODE_SHIFT = 26;
    static constexpr uint32_t C_SHIFT = 0;
    static constexpr uint32_t SBX_SHIFT = 0;
    static constexpr uint32_t J_ZERO = BX_ARG >> 1;
    static constexpr int VM_NORMAL = 0;
    static constexpr int VM_RECORD = 1;

    static constexpr int HOT_THRESHOLD = 3;

    // template<uint16_t GC_YOUNG_THRESHOLD=50>
    struct VMData {
        heap::GarbageCollector<16 + 4 + 1> gc{};

        //  Static data: must be filled before running vm
        std::vector<Value> constanti;
        std::vector<Value> constantf;
        std::vector<ObjClass> classes;
        Function functions[FUNCTIONS_MAX];
        NativeFunction natives[FUNCTIONS_MAX];
        // -- general info --
        size_t code_size = 0;
        size_t functions_count = 0;

        // Heap storage

        // std::shared_ptr<Value[]> heap[HEAP_MAX_SIZE];
        // std::vector<std::shared_ptr<Value[]>>& heap = heap::get_heap();

        // uint32_t heap_size = 0;

        // Execution state
        Value stack[STACK_SIZE];
        uint32_t code[CODE_MAX_SIZE];

        uint32_t ip = 0;  // Instruction pointer
        uint32_t sp = 0;  // Stack pointer
        uint32_t fp = 0;  // Frame pointer
        std::stack<CallFrame> call_stack;

        // -- JIT data --
//        util::int_ptr_map trace_head;
        jit::Trace *trace = nullptr;


    };

// Core VM functions
    void run(bool with_gc = true);

    VMData &vm_instance();

// Helper functions (creating opcode)

// For: OP_LOAD, OP_NEWOBJ
    uint32_t opcode(OpCode code, uint8_t a, uint32_t bx);

// For: OP_MOVE(!better use move() to not mistake), OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
//                OP_EQ, OP_LT, OP_LE, OP_GETFIELD, OP_SETFIELD
    uint32_t opcode(OpCode code, uint8_t a, uint8_t b, uint8_t c);

// For: OP_RETURN
//      OP_LOADNIL
//      OP_NEG
    uint32_t opcode(OpCode code, uint8_t a);

// For: OP_RETURNNIL, OP_HALT
    uint32_t opcode(OpCode code);

    uint32_t halt();

    uint32_t move(uint8_t a, uint8_t b);

// Printing opcode
    void print_opcode(uint32_t instruction);

    Value add_values(const Value &a, const Value &b);

    Value sub_values(const Value &a, const Value &b);

    Value mul_values(const Value &a, const Value &b);

    Value div_values(const Value &a, const Value &b);

    bool is_truthy(const Value &val);

    // using VMData = VMData<>

// Instruction implementations
    void op_load(VMData &vm, uint8_t reg, uint32_t const_idx);

    void op_move(VMData &vm, uint8_t dst, uint8_t src);

    void op_loadnil(VMData &vm, uint8_t reg);

    void op_loadint(VMData &vm, uint8_t reg, uint32_t const_idx);

    void op_loadfloat(VMData &vm, uint8_t reg, uint32_t const_idx);

    void op_loadfunc(VMData &vm, uint8_t reg, uint32_t const_idx);

    void op_alloc(VMData &vm, uint8_t dst, uint8_t size);

    void op_arrget(VMData &vm, uint8_t dst, uint8_t arr, uint8_t idx);

    void op_arrset(VMData &vm, uint8_t arr, uint8_t idx, uint8_t src);

// void op_tailcall(VMData &vm, uint8_t func_idx, uint8_t first_arg_ind, uint8_t num_args);
    void op_add(VMData &vm, uint8_t dst, uint8_t src1, uint8_t src2);

    void op_sub(VMData &vm, uint8_t dst, uint8_t src1, uint8_t src2);

    void op_mul(VMData &vm, uint8_t dst, uint8_t src1, uint8_t src2);

    void op_div(VMData &vm, uint8_t dst, uint8_t src1, uint8_t src2);

    void op_mod(VMData &vm, uint8_t dst, uint8_t src1, uint8_t src2);

    void op_neg(VMData &vm, uint8_t dst, uint8_t src);

    void op_eq(VMData &vm, uint8_t dst, uint8_t src1, uint8_t src2);

    void op_neq(VMData &vm, uint8_t dst, uint8_t src1, uint8_t src2);

    void op_lt(VMData &vm, uint8_t dst, uint8_t src1, uint8_t src2);

    void op_le(VMData &vm, uint8_t dst, uint8_t src1, uint8_t src2);

    void op_call(VMData &vm, uint8_t func_idx, uint8_t first_arg_ind, uint8_t num_args);

    void op_native_call(VMData &vm, uint8_t func_idx, int reg1, int count);

    void op_invokedyn(VMData &vm, uint8_t a, uint8_t b, uint8_t c);

    void op_return(VMData &vm, uint8_t result_reg);

    void op_returnnil(VMData &vm);

    void op_newobj(VMData &vm, uint8_t dst, uint32_t class_idx);

    void op_getfield(VMData &vm, uint8_t dst, uint8_t obj, uint8_t field_idx);

    void op_setfield(VMData &vm, uint8_t obj, uint8_t field_idx, uint8_t src);

    void op_halt(VMData &vm);

    void op_jmp(VMData &vm, int32_t offset);

    void op_jmpt(VMData &vm, uint8_t cond, int32_t offset);

    void op_jmpf(VMData &vm, uint8_t cond, int32_t offset);

};  // namespace interpreter

#endif  // CRYPT_VM_H