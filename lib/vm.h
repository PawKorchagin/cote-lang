#ifndef CRYPT_VM_H
#define CRYPT_VM_H

#include <cstdint>
#include <fstream>
#include <stack>
#include <unordered_map>
#include <vector>

namespace interpreter {
    struct VMData;

    enum OpCode {
        // Loads a constant into a register
        // Args: a - destination register, bx - constant pool index
        // Behavior: registers[a] = constants[bx]
        OP_LOAD,

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
        OP_EQ,

        // Args: a - result register, b - first operand, c - second operand
        // Behavior: registers[a] = (registers[b] != registers[c]) ? 1 : 0
        OP_NEQ,

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
        OP_JMPT,

        // Jump if false
        // Args: Same as OP_JMPT
        // Behavior: if (!registers[a]) ip += sbx
        OP_JMPF,

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
        OP_NATIVE_CALL,

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

        // Creates new object
        // Args: a - destination register, bx - class index
        // Behavior:
        //   1. Allocates heap memory
        //   2. Initializes fields
        //   3. registers[a] = object reference
        OP_NEWOBJ,

        // Reads object field
        // Args: a - destination, b - object register, c - field index
        // Behavior: registers[a] = object(b).fields[c]
        OP_GETFIELD,

        // Writes object field
        // Args: a - object register, b - field index, c - value register
        // Behavior: object(a).fields[b] = registers[c]
        OP_SETFIELD,

        // Stops execution
        // Args: none
        // Behavior: terminates VM execution
        OP_HALT
    };

    enum class ValueType : uint8_t { Nil, Int, Float, Char, Object, Callable };

    struct Value {
        ValueType type = ValueType::Nil;
        uint16_t class_ptr = 0; //only for objects
        union {
            int32_t i32;
            float f32;
            char c;
            uint8_t callable;
            uint32_t object_ptr{};
        } as{};

        bool is_nil() const { return type == ValueType::Nil; }
        bool is_int() const { return type == ValueType::Int; }
        bool is_float() const { return type == ValueType::Float; }
        bool is_char() const { return type == ValueType::Char; }
        bool is_object() const { return type == ValueType::Object; }
        bool is_callable() const { return type == ValueType::Callable; }
    };

    struct Object {
        std::vector<Value> fields;
    };

    struct Function {
        uint32_t entry_point;
        uint8_t arity;
        uint8_t local_count;
    };

    typedef void(*NativeFunction)(VMData&);
    struct ObjClass {
        std::pmr::unordered_map<std::string, int> indexes;
    };

// Memory limits
    static constexpr uint32_t CODE_MAX_SIZE = 4096;
    static constexpr uint32_t STACK_SIZE    = 4096;
    static constexpr uint32_t HEAP_MAX_SIZE = 65536;
    static constexpr uint32_t FUNCTIONS_MAX = 256;
    static constexpr uint32_t CALL_MAX_SIZE = 2000;

// Dispatch constants
    static constexpr uint32_t A_ARG        = 0xFF;
    static constexpr uint32_t A_SHIFT      = 18;
    static constexpr uint32_t B_ARG        = 0x1FF;
    static constexpr uint32_t B_SHIFT      = 9;
    static constexpr uint32_t C_ARG        = 0x1FF;
    static constexpr uint32_t BX_ARG       = 0x3FFFF;
    static constexpr uint32_t OPCODE_SHIFT = 26;
    static constexpr uint32_t C_SHIFT      = 0;
    static constexpr uint32_t SBX_SHIFT    = 0;
    static constexpr uint32_t J_ZERO       = BX_ARG >> 1;

    struct CallFrame {
        uint32_t return_ip;
        uint32_t base_ptr;
    };

    struct VMData {
        //--for debug---
        int code_size = 0;
        //  Static data: must be filled before running vm
        std::vector<Value> constants;
        std::vector<ObjClass> classes;
        Function functions[FUNCTIONS_MAX];
        NativeFunction natives[FUNCTIONS_MAX];

        // Heap storage
        Object* heap[HEAP_MAX_SIZE];
        uint32_t heap_size = 0;

        // Execution state
        Value stack[STACK_SIZE];
        uint32_t code[CODE_MAX_SIZE];

        uint32_t ip = 0;  // Instruction pointer
        uint32_t sp = 0;  // Stack pointer
        uint32_t fp = 0;  // Frame pointer
        std::stack<CallFrame> call_stack;
    };

// Core VM functions
    void run();
    VMData& vm_instance();

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
    uint32_t jmp(int32_t offset);
    uint32_t jmpt(uint8_t a, int32_t offset);
    uint32_t jmpf(uint8_t a, int32_t offset);
    uint32_t move(uint8_t a, uint8_t b);

// Printing opcode
    void print_opcode(uint32_t instruction);

    Value add_values(const Value& a, const Value& b);
    Value sub_values(const Value& a, const Value& b);
    Value mul_values(const Value& a, const Value& b);
    Value div_values(const Value& a, const Value& b);
    bool is_truthy(const Value& val);
    Value convert_value(const Value& val, ValueType target_type);

// Instruction implementations
    void op_load(VMData& vm, uint8_t reg, uint32_t const_idx);
    void op_move(VMData& vm, uint8_t dst, uint8_t src);
    void op_loadnil(VMData& vm, uint8_t reg);
    void op_add(VMData& vm, uint8_t dst, uint8_t src1, uint8_t src2);
    void op_sub(VMData& vm, uint8_t dst, uint8_t src1, uint8_t src2);
    void op_mul(VMData& vm, uint8_t dst, uint8_t src1, uint8_t src2);
    void op_div(VMData& vm, uint8_t dst, uint8_t src1, uint8_t src2);
    void op_mod(VMData& vm, uint8_t dst, uint8_t src1, uint8_t src2);
    void op_neg(VMData& vm, uint8_t dst, uint8_t src);
    void op_eq(VMData& vm, uint8_t dst, uint8_t src1, uint8_t src2);
    void op_neq(VMData& vm, uint8_t dst, uint8_t src1, uint8_t src2);
    void op_lt(VMData& vm, uint8_t dst, uint8_t src1, uint8_t src2);
    void op_le(VMData& vm, uint8_t dst, uint8_t src1, uint8_t src2);
    void op_jmp(VMData& vm, int32_t offset);
    void op_jmpt(VMData& vm, uint8_t cond, int32_t offset);
    void op_jmpf(VMData& vm, uint8_t cond, int32_t offset);
    void op_call(VMData& vm, uint8_t func_idx, uint8_t first_arg_ind, uint8_t num_args);
    void op_native_call(VMData& vm, uint8_t func_idx);
    void op_invokedyn(VMData& vm, uint8_t a, uint8_t b, uint8_t c);
    void op_return(VMData& vm, uint8_t result_reg);
    void op_returnnil(VMData& vm);
    void op_newobj(VMData& vm, uint8_t dst, uint32_t class_idx);
    void op_getfield(VMData& vm, uint8_t dst, uint8_t obj, uint8_t field_idx);
    void op_setfield(VMData& vm, uint8_t obj, uint8_t field_idx, uint8_t src);
    void op_halt(VMData& vm);

};  // namespace interpreter

#endif  // CRYPT_VM_H