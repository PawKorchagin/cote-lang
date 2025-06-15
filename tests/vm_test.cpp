//
// Created by motya on 12.04.2025.
//
#include "../lib/vm.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

#include "utils.h"

using namespace interpreter;

struct BytecodeHolder {
    std::unordered_map<std::string, int> labels;
    std::vector<std::pair<std::string, int>> pending_labels;
    std::vector<uint32_t> code;

    BytecodeHolder() = default;

    BytecodeHolder& emit(uint32_t instr) {
        code.push_back(instr);
        return *this;
    }

    BytecodeHolder& label(std::string label) {
        if (labels.count(label))
            throw std::runtime_error("label already exists");
        labels[label] = static_cast<int>(code.size());
        return *this;
    }

    BytecodeHolder& jmp(OpCode jmpType, std::string label) {
        pending_labels.emplace_back(label, static_cast<int>(code.size()));
        return emit(jmpType << 26);
    }

    BytecodeHolder& update() {
        for (auto& [label, pos] : pending_labels) {
            if (!labels.count(label))
                throw std::runtime_error("label not found " + label);
            int target = labels[label];
            int offset = target - pos - 1;
            code[pos] |= (offset & 0x3FFFFFF);
        }
        pending_labels.clear();
        return *this;
    }

    BytecodeHolder& emit(OpCode op, uint8_t a, uint8_t b, uint8_t c) {
        return emit((op << 26) | (a << 18) | (b << 9) | c);
    }

    BytecodeHolder& emit(OpCode op, uint8_t a, uint32_t bx) { return emit((op << 26) | (a << 18) | (bx & 0x3FFFF)); }
};

template <class T>
void run_checked(const std::vector<Value>& constants, BytecodeHolder& h, T check) {
    VMData& vm = vm_instance();
    vm         = VMData();  // Reset VM

    // Setup constants
    vm.constants = constants;

    // Copy bytecode
    h.emit(OP_HALT << 26);  // Ensure halt at end
    if (h.code.size() > CODE_MAX_SIZE) {
        throw std::runtime_error("Code size exceeds maximum");
    }
    std::copy(h.code.begin(), h.code.end(), vm.code);

    // Execute
    vm.ip = 0;
    run();

    // Verify
    check(vm);
}

// Value creation helpers
Value int_val(int32_t v) {
    Value val;
    val.type   = ValueType::Int;
    val.as.i32 = v;
    return val;
}
Value float_val(float v) {
    Value val;
    val.type   = ValueType::Float;
    val.as.f32 = v;
    return val;
}
Value char_val(char v) {
    Value val;
    val.type = ValueType::Char;
    val.as.c = v;
    return val;
}
Value nil_val() {
    Value val;
    val.type = ValueType::Nil;
    return val;
}

TEST(OpcodeCreationTest, OpcodeTest) {
    VMData& vm = initVM();

    vm.constants = {int_val(10), int_val(20), int_val(30)};

    vm.code[0] = opcode(OP_LOAD, 1, 0);     // R1 = C0
    vm.code[1] = opcode(OP_LOAD, 2, 1);     // R2 = C1
    vm.code[2] = opcode(OP_MOVE, 1, 2, 0);  // R1 = R2
    vm.code[3] = jmp(1);                         // ip += 1
    vm.code[3] = opcode(OP_LOAD, 1, 3);     // R1 = C3 skip
    vm.code[3] = halt();                              //end

    run();

    ASSERT_EQ(vm.stack[vm.fp + 1].as.i32, 20);
    ASSERT_EQ(vm.stack[vm.fp + 1].type, ValueType::Int);
}
TEST(VmLoadTest, BasicOperation) {
    VMData& vm = initVM();

    vm.constants = {int_val(10), int_val(20), int_val(30)};

    vm.code[0] = (OP_LOAD << OPCODE_SHIFT) | (1 << A_SHIFT) | 0;
    vm.code[1] = (OP_HALT << OPCODE_SHIFT);

    run();

    ASSERT_EQ(vm.stack[vm.fp + 1].as.i32, 10);
    ASSERT_EQ(vm.stack[vm.fp + 1].type, ValueType::Int);
}

TEST(VmMoveTest, BasicOperation) {
    VMData& vm = initVM();

    vm.constants = {int_val(10), int_val(20), int_val(30)};

    vm.code[0] = (OP_LOAD << OPCODE_SHIFT) | (1 << A_SHIFT) | 0;
    vm.code[1] = (OP_LOAD << OPCODE_SHIFT) | (2 << A_SHIFT) | 1;
    vm.code[2] = (OP_MOVE << OPCODE_SHIFT) | (1 << A_SHIFT) | (2 << B_SHIFT);
    vm.code[3] = (OP_HALT << OPCODE_SHIFT);

    run();

    ASSERT_EQ(vm.stack[vm.fp + 1].as.i32, 20);
    ASSERT_EQ(vm.stack[vm.fp + 1].type, ValueType::Int);
}

TEST(VmArithmeticTest, AdditionOperation) {
    VMData& vm = initVM();

    vm.constants = {int_val(10), int_val(20)};

    vm.code[0] = (OP_LOAD << OPCODE_SHIFT) | (1 << A_SHIFT) | 0;                  // R1 = 10
    vm.code[1] = (OP_LOAD << OPCODE_SHIFT) | (2 << A_SHIFT) | 1;                  // R2 = 20
    vm.code[2] = (OP_ADD << OPCODE_SHIFT) | (3 << A_SHIFT) | (1 << B_SHIFT) | 2;  // R3 = R1 + R2
    vm.code[3] = (OP_HALT << OPCODE_SHIFT);

    run();

    ASSERT_EQ(vm.stack[vm.fp + 3].as.i32, 30);
    ASSERT_EQ(vm.stack[vm.fp + 3].type, ValueType::Int);
}
TEST(VmLoadNilTest, BasicOperation) {
    VMData& vm = initVM();

    vm.code[0] = (OP_LOADNIL << OPCODE_SHIFT) | (1 << A_SHIFT);
    vm.code[1] = (OP_HALT << OPCODE_SHIFT);

    run();

    ASSERT_EQ(vm.stack[vm.fp + 1].type, ValueType::Nil);
    ASSERT_TRUE(vm.stack[vm.fp + 1].is_nil());
}

TEST(VmArithmeticTest, SubtractionOperation) {
    VMData& vm = initVM();

    vm.constants = {int_val(30), int_val(10)};

    vm.code[0] = (OP_LOAD << OPCODE_SHIFT) | (1 << A_SHIFT) | 0;                               // R1 = 30
    vm.code[1] = (OP_LOAD << OPCODE_SHIFT) | (2 << A_SHIFT) | 1;                               // R2 = 10
    vm.code[2] = (OP_SUB << OPCODE_SHIFT) | (3 << A_SHIFT) | (1 << B_SHIFT) | (2 << C_SHIFT);  // R3 = R1 - R2
    vm.code[3] = (OP_HALT << OPCODE_SHIFT);

    run();

    ASSERT_EQ(vm.stack[vm.fp + 3].as.i32, 20);
    ASSERT_EQ(vm.stack[vm.fp + 3].type, ValueType::Int);
}

TEST(VmArithmeticTest, MultiplicationOperation) {
    VMData& vm = initVM();

    vm.constants = {int_val(5), int_val(6)};

    vm.code[0] = (OP_LOAD << OPCODE_SHIFT) | (1 << A_SHIFT) | 0;                               // R1 = 5
    vm.code[1] = (OP_LOAD << OPCODE_SHIFT) | (2 << A_SHIFT) | 1;                               // R2 = 6
    vm.code[2] = (OP_MUL << OPCODE_SHIFT) | (3 << A_SHIFT) | (1 << B_SHIFT) | (2 << C_SHIFT);  // R3 = R1 * R2
    vm.code[3] = (OP_HALT << OPCODE_SHIFT);

    run();

    ASSERT_EQ(vm.stack[vm.fp + 3].as.i32, 30);
    ASSERT_EQ(vm.stack[vm.fp + 3].type, ValueType::Int);
}

TEST(VmArithmeticTest, DivisionOperation) {
    VMData& vm = initVM();

    vm.constants = {int_val(20), int_val(5)};

    vm.code[0] = (OP_LOAD << OPCODE_SHIFT) | (1 << A_SHIFT) | 0;                               // R1 = 20
    vm.code[1] = (OP_LOAD << OPCODE_SHIFT) | (2 << A_SHIFT) | 1;                               // R2 = 5
    vm.code[2] = (OP_DIV << OPCODE_SHIFT) | (3 << A_SHIFT) | (1 << B_SHIFT) | (2 << C_SHIFT);  // R3 = R1 / R2
    vm.code[3] = (OP_HALT << OPCODE_SHIFT);

    run();

    ASSERT_EQ(vm.stack[vm.fp + 3].as.i32, 4);
    ASSERT_EQ(vm.stack[vm.fp + 3].type, ValueType::Int);
}

TEST(VmArithmeticTest, ModuloOperation) {
    VMData& vm = initVM();

    vm.constants = {int_val(10), int_val(3)};

    vm.code[0] = (OP_LOAD << OPCODE_SHIFT) | (1 << A_SHIFT) | 0;                               // R1 = 10
    vm.code[1] = (OP_LOAD << OPCODE_SHIFT) | (2 << A_SHIFT) | 1;                               // R2 = 3
    vm.code[2] = (OP_MOD << OPCODE_SHIFT) | (3 << A_SHIFT) | (1 << B_SHIFT) | (2 << C_SHIFT);  // R3 = R1 % R2
    vm.code[3] = (OP_HALT << OPCODE_SHIFT);

    run();

    ASSERT_EQ(vm.stack[vm.fp + 3].as.i32, 1);
    ASSERT_EQ(vm.stack[vm.fp + 3].type, ValueType::Int);
}

TEST(VmArithmeticTest, NegationOperation) {
    VMData& vm = initVM();

    vm.constants = {int_val(42)};

    vm.code[0] = (OP_LOAD << OPCODE_SHIFT) | (1 << A_SHIFT) | 0;              // R1 = 42
    vm.code[1] = (OP_NEG << OPCODE_SHIFT) | (2 << A_SHIFT) | (1 << B_SHIFT);  // R2 = -R1
    vm.code[2] = (OP_HALT << OPCODE_SHIFT);

    run();

    ASSERT_EQ(vm.stack[vm.fp + 2].as.i32, -42);
    ASSERT_EQ(vm.stack[vm.fp + 2].type, ValueType::Int);
}

TEST(VmComparisonTest, EqualityOperation) {
    VMData& vm = initVM();

    vm.constants = {int_val(10), int_val(10), int_val(20)};

    vm.code[0] = (OP_LOAD << OPCODE_SHIFT) | (1 << A_SHIFT) | 0;                              // R1 = 10
    vm.code[1] = (OP_LOAD << OPCODE_SHIFT) | (2 << A_SHIFT) | 1;                              // R2 = 10
    vm.code[2] = (OP_LOAD << OPCODE_SHIFT) | (3 << A_SHIFT) | 2;                              // R3 = 20
    vm.code[3] = (OP_EQ << OPCODE_SHIFT) | (4 << A_SHIFT) | (1 << B_SHIFT) | (2 << C_SHIFT);  // R4 = (R1 == R2)
    vm.code[4] = (OP_EQ << OPCODE_SHIFT) | (5 << A_SHIFT) | (1 << B_SHIFT) | (3 << C_SHIFT);  // R5 = (R1 == R3)
    vm.code[5] = (OP_HALT << OPCODE_SHIFT);

    run();

    ASSERT_EQ(vm.stack[vm.fp + 4].as.i32, 1);  // 10 == 10
    ASSERT_EQ(vm.stack[vm.fp + 5].as.i32, 0);  // 10 != 20
}

TEST(VmComparisonTest, NEqualityOperation) {
    VMData& vm = initVM();

    vm.constants = {int_val(10), int_val(10), int_val(20)};

    vm.code[0] = (OP_LOAD << OPCODE_SHIFT) | (1 << A_SHIFT) | 0;                              // R1 = 10
    vm.code[1] = (OP_LOAD << OPCODE_SHIFT) | (2 << A_SHIFT) | 1;                              // R2 = 10
    vm.code[2] = (OP_LOAD << OPCODE_SHIFT) | (3 << A_SHIFT) | 2;                              // R3 = 20
    vm.code[3] = (OP_NEQ << OPCODE_SHIFT) | (4 << A_SHIFT) | (1 << B_SHIFT) | (2 << C_SHIFT);  // R4 = (R1 != R2)
    vm.code[4] = (OP_NEQ << OPCODE_SHIFT) | (5 << A_SHIFT) | (1 << B_SHIFT) | (3 << C_SHIFT);  // R5 = (R1 != R3)
    vm.code[5] = (OP_HALT << OPCODE_SHIFT);

    run();

    ASSERT_EQ(vm.stack[vm.fp + 4].as.i32, 0);  // 10 == 10
    ASSERT_EQ(vm.stack[vm.fp + 5].as.i32, 1);  // 10 != 20
}

TEST(VmComparisonTest, LessThanOperation) {
    VMData& vm = initVM();

    vm.constants = {int_val(10), int_val(20)};

    vm.code[0] = (OP_LOAD << OPCODE_SHIFT) | (1 << A_SHIFT) | 0;                              // R1 = 10
    vm.code[1] = (OP_LOAD << OPCODE_SHIFT) | (2 << A_SHIFT) | 1;                              // R2 = 20
    vm.code[2] = (OP_LT << OPCODE_SHIFT) | (3 << A_SHIFT) | (1 << B_SHIFT) | (2 << C_SHIFT);  // R3 = (R1 < R2)
    vm.code[3] = (OP_LT << OPCODE_SHIFT) | (4 << A_SHIFT) | (2 << B_SHIFT) | (1 << C_SHIFT);  // R4 = (R2 < R1)
    vm.code[4] = (OP_HALT << OPCODE_SHIFT);

    run();

    ASSERT_EQ(vm.stack[vm.fp + 3].as.i32, 1);  // 10 < 20
    ASSERT_EQ(vm.stack[vm.fp + 4].as.i32, 0);  // 20 not < 10
}

TEST(VmJumpTest, UnconditionalJump) {
    VMData& vm = initVM();

    vm.constants = {int_val(1), int_val(2)};

    // This test jumps over the second LOAD instruction
    vm.code[0] = (OP_LOAD << OPCODE_SHIFT) | (1 << A_SHIFT) | 0;          // R1 = 1
    vm.code[1] = (OP_JMP << OPCODE_SHIFT) | ((1 + J_ZERO) << SBX_SHIFT);  // Jump +2
    vm.code[2] = (OP_LOAD << OPCODE_SHIFT) | (2 << A_SHIFT) | 1;          // R2 = 2 (should be skipped)
    vm.code[3] = (OP_HALT << OPCODE_SHIFT);

    run();

    // Only R1 should be loaded
    ASSERT_EQ(vm.stack[vm.fp + 1].as.i32, 1);
    ASSERT_EQ(vm.stack[vm.fp + 2].type, ValueType::Nil);  // R2 wasn't loaded
}

TEST(VmJumpTest, ConditionalJump) {
    VMData& vm = initVM();

    vm.constants = {int_val(1), int_val(0), int_val(42)};

    // Test both JMPT and JMPF
    vm.code[0] = (OP_LOAD << OPCODE_SHIFT) | (1 << A_SHIFT) | 0;                            // R1 = 1 (true)
    vm.code[1] = (OP_LOAD << OPCODE_SHIFT) | (2 << A_SHIFT) | 1;                            // R2 = 0 (false)
    vm.code[2] = (OP_JMPT << OPCODE_SHIFT) | (1 << A_SHIFT) | ((1 + J_ZERO) << SBX_SHIFT);  // if (R1) jump +2
    vm.code[3] = (OP_HALT << OPCODE_SHIFT);                                                 // Should be skipped
    vm.code[4] = (OP_JMPF << OPCODE_SHIFT) | (2 << A_SHIFT) | ((1 + J_ZERO) << SBX_SHIFT);  // if (!R2) jump +2
    vm.code[5] = (OP_HALT << OPCODE_SHIFT);                                                 // Should be skipped
    vm.code[6] = (OP_LOAD << OPCODE_SHIFT) | (3 << A_SHIFT) | 2;                            // R3 = 42
    vm.code[7] = (OP_HALT << OPCODE_SHIFT);

    run();

    ASSERT_EQ(vm.stack[vm.fp + 3].as.i32, 42);  // Should reach this
}

TEST(VmObjectTest, BasicObjectOperations) {
    VMData& vm = initVM();

    // Define a simple object context with 3 fields
    auto obj = ObjClass{};
    obj.indexes =  {{"x", 1}, {"y", 2}, {"z", 3}};
    vm.classes = {obj};

    vm.constants = {int_val(10), int_val(20)};

    vm.code[0] = (OP_NEWOBJ << OPCODE_SHIFT) | (1 << A_SHIFT) | 0;  // R1 = new object (context 0)
    vm.code[1] = (OP_LOAD << OPCODE_SHIFT) | (2 << A_SHIFT) | 0;    // R2 = 10 (from constants)
    vm.code[2] = (OP_LOAD << OPCODE_SHIFT) | (3 << A_SHIFT) | 1;    // R3 = 20

    // Set fields
    vm.code[3] = (OP_SETFIELD << OPCODE_SHIFT) | (1 << A_SHIFT) | (0 << B_SHIFT) | (2 << C_SHIFT);  // obj.field0 = R2
    vm.code[4] = (OP_SETFIELD << OPCODE_SHIFT) | (1 << A_SHIFT) | (1 << B_SHIFT) | (3 << C_SHIFT);  // obj.field1 = R3

    // Get fields back
    vm.code[5] = (OP_GETFIELD << OPCODE_SHIFT) | (4 << A_SHIFT) | (1 << B_SHIFT) | (0 << C_SHIFT);  // R4 = obj.field0
    vm.code[6] = (OP_GETFIELD << OPCODE_SHIFT) | (5 << A_SHIFT) | (1 << B_SHIFT) | (1 << C_SHIFT);  // R5 = obj.field1

    vm.code[7] = (OP_HALT << OPCODE_SHIFT);

    run();

    // Verify object operations
    ASSERT_EQ(vm.stack[vm.fp + 1].type, ValueType::Object);
    ASSERT_EQ(vm.stack[vm.fp + 1].as.object_ptr, 0);
    ASSERT_EQ(vm.classes[vm.stack[vm.fp + 1].class_ptr].indexes["x"], 1);

    // Verify field values
    ASSERT_EQ(vm.stack[vm.fp + 4].as.i32, 10);
    ASSERT_EQ(vm.stack[vm.fp + 5].as.i32, 20);
}

TEST(VmFunctionTest, BasicFunctionCall) {
    VMData& vm = initVM();

    // Define a simple function that adds two numbers
    Function add_func{};
    add_func.entry_point = 5;  // Points to the ADD instruction
    add_func.arity       = 2;
    add_func.local_count = 3;  // 2 args + 1 local

    vm.functions[0] = add_func;

    // Main code
    vm.constants = {int_val(10), int_val(20)};
    vm.code[0]   = (OP_LOAD << OPCODE_SHIFT) | (1 << A_SHIFT) | 0;               // R1 = 10 (arg1)
    vm.code[1]   = (OP_LOAD << OPCODE_SHIFT) | (2 << A_SHIFT) | 1;               // R2 = 20 (arg2)
    // call func 0 with args from R(1) to R(2)
    vm.code[2]   = opcode(OP_CALL, 0, 1, 2);
    // call 2 times to ensure that sp not changed
    vm.code[3]   = opcode(OP_CALL, 0, 1, 2);
    vm.code[4]   = (OP_HALT << OPCODE_SHIFT);

    // Function code
    vm.code[5] = (OP_ADD << OPCODE_SHIFT) | (0 << A_SHIFT) | (0 << B_SHIFT) | (1 << C_SHIFT);  // R0 = R1 + R2
    vm.code[6] = (OP_RETURN << OPCODE_SHIFT) | (0 << A_SHIFT);                                 // return R0

    run();

    //last valid value in R2
    // ASSERT_EQ(vm.sp, 2);

    // The result should be in R0 (30)
    ASSERT_EQ(vm.stack[1].as.i32, 50);
}
TEST(VmFunctionTest, StackPointerManagement) {
    VMData& vm = initVM();

    // Function that uses all its locals
    Function func{};
    func.entry_point = 5;
    func.arity = 2;
    func.local_count = 4;  // 2 args + 2 locals

    vm.functions[0] = func;

    // Main code
    vm.constants = {int_val(10), int_val(20), int_val(0)}; // 0 will be used as nil
    vm.code[0] = opcode(OP_LOAD, 1, 0);  // R1 = 10
    vm.code[1] = opcode(OP_LOAD, 2, 1);  // R2 = 20
    vm.code[2] = opcode(OP_CALL, 0, 1, 2); // call func(10,20)
    vm.code[3] = opcode(OP_HALT);

    // Function code - uses all locals
    vm.code[5] = opcode(OP_LOADNIL, 3);   // R3 = nil
    vm.code[6] = opcode(OP_LOADNIL, 4);   // R4 = nil
    vm.code[7] = opcode(OP_RETURNNIL);    // return nil

    run();

    // ASSERT_EQ(vm.sp, 2);
    ASSERT_TRUE(vm.call_stack.empty());
}

TEST(VmFunctionTest, RecursiveFunction) {
    VMData& vm = initVM();

    // Recursive function to calculate sum from n to 1
    Function sum_func{};
    sum_func.entry_point = 5;
    sum_func.arity = 1;
    sum_func.local_count = 2;  // 1 arg + 1 local

    vm.functions[0] = sum_func;

    // Main code and constants
    vm.constants = {int_val(1), int_val(5)}; // 0=1, 1=5
    vm.code[0] = opcode(OP_LOAD, 1, 1);  // R1 = 5
    vm.code[1] = opcode(OP_CALL, 0, 1, 1); // sum(5)
    vm.code[2] = opcode(OP_HALT);

    // Function code
    // if (n <= 1) return 1
    vm.code[5] = opcode(OP_LOAD, 2, 0);  // R2 = 1
    vm.code[6] = opcode(OP_LE, 3, 1, 2); // R3 = (R1 <= 1)
    vm.code[7] = jmpf(3, 3);  // if !R3 jump +3
    vm.code[8] = opcode(OP_LOAD, 0, 0);  // R0 = 1
    vm.code[9] = opcode(OP_RETURN, 0);   // return 1
    // else return n + sum(n-1)
    vm.code[10] = opcode(OP_LOAD, 2, 0); // R2 = 1
    vm.code[11] = opcode(OP_SUB, 4, 1, 2); // R4 = n-1
    vm.code[12] = opcode(OP_CALL, 0, 4, 1); // sum(n-1)
    vm.code[13] = opcode(OP_ADD, 0, 1, 0); // R0 = n + sum(n-1)
    vm.code[14] = opcode(OP_RETURN, 0);    // return R0

    run();

    ASSERT_EQ(vm.stack[0].as.i32, 15);
    ASSERT_TRUE(vm.call_stack.empty());
}

TEST(VmFunctionTest, Factorial) {
    VMData& vm = initVM();

    // Factorial function
    Function fact_func{};
    fact_func.entry_point = 5;
    fact_func.arity = 1;
    fact_func.local_count = 2;  // 1 arg + 1 local

    vm.functions[0] = fact_func;

    // Main code and constants
    vm.constants = {int_val(1), int_val(6)}; // 0=1, 1=6
    vm.code[0] = opcode(OP_LOAD, 1, 1);  // R1 = 6
    vm.code[1] = opcode(OP_CALL, 0, 1, 1); // fact(6)
    vm.code[2] = opcode(OP_HALT);

    // Function code
    // if (n <= 1) return 1
    vm.code[5] = opcode(OP_LOAD, 2, 0);  // R2 = 1
    vm.code[6] = opcode(OP_LE, 3, 1, 2); // R3 = (R1 <= 1)
    vm.code[7] = jmpf(3, 3);  // if !R3 jump +3
    vm.code[8] = opcode(OP_LOAD, 0, 0);  // R0 = 1
    vm.code[9] = opcode(OP_RETURN, 0);   // return 1
    // else return n * fact(n-1)
    vm.code[10] = opcode(OP_LOAD, 2, 0); // R2 = 1
    vm.code[11] = opcode(OP_SUB, 4, 1, 2); // R4 = n-1
    vm.code[12] = opcode(OP_CALL, 0, 4, 1); // fact(n-1)
    vm.code[13] = opcode(OP_MUL, 0, 1, 0); // R0 = n * fact(n-1)
    vm.code[14] = opcode(OP_RETURN, 0);    // return R0

    run();

    ASSERT_EQ(vm.stack[0].as.i32, 720);
    ASSERT_TRUE(vm.call_stack.empty());
}

TEST(VmFunctionTest, VoidFunction) {
    VMData& vm = initVM();

    // Function that does nothing
    Function void_func{};
    void_func.entry_point = 5;
    void_func.arity = 0;
    void_func.local_count = 0;  // just R0

    vm.functions[0] = void_func;

    // Main code
    vm.code[0] = opcode(OP_CALL, 0, 0, 0); // call void_func()
    vm.code[1] = opcode(OP_HALT);

    // Function code
    vm.code[5] = opcode(OP_RETURNNIL); // return nil

    run();

    ASSERT_EQ(vm.stack[0].type, ValueType::Nil);
    // ASSERT_EQ(vm.sp, 0);
    ASSERT_TRUE(vm.call_stack.empty());
}

TEST(VmFunctionTest, NoArgsFunction) {
    VMData& vm = initVM();

    // Function that returns constant value
    Function const_func{};
    const_func.entry_point = 5;
    const_func.arity = 0;
    const_func.local_count = 0;  // just R0

    vm.functions[0] = const_func;
    vm.constants = {int_val(42)};

    // Main code
    vm.code[0] = opcode(OP_CALL, 0, 1, 0); // call const_func()
    vm.code[1] = opcode(OP_HALT);

    // Function code
    vm.code[5] = opcode(OP_LOAD, 0, 0); // R0 = 42
    vm.code[6] = opcode(OP_RETURN, 0);   // return R0

    run();

    ASSERT_EQ(vm.stack[0].as.i32, 42);
    // ASSERT_EQ(vm.sp, 0);
    ASSERT_TRUE(vm.call_stack.empty());
}

TEST(VmFunctionTest, NestedCalls) {
    VMData& vm = initVM();

    // Function A calls function B
    Function func_a{};
    func_a.entry_point = 5;
    func_a.arity = 1;
    func_a.local_count = 2;

    Function func_b{};
    func_b.entry_point = 10;
    func_b.arity = 1;
    func_b.local_count = 2;

    vm.functions[0] = func_a;
    vm.functions[1] = func_b;
    vm.constants = {int_val(10)};

    // Main code
    vm.code[0] = opcode(OP_LOAD, 1, 0);  // R1 = 10
    vm.code[1] = opcode(OP_CALL, 0, 1, 1); // call A(10)
    vm.code[2] = opcode(OP_HALT);

    // Function A code
    vm.code[5] = opcode(OP_CALL, 1, 1, 1); // call B(arg)
    vm.code[6] = opcode(OP_ADD, 0, 1, 0);  // R0 = arg + B(arg)
    vm.code[7] = opcode(OP_RETURN, 0);      // return R0

    // Function B code
    vm.code[10] = opcode(OP_LOAD, 2, 0);   // R2 = 10
    vm.code[11] = opcode(OP_ADD, 0, 1, 2);  // R0 = arg + 10
    vm.code[12] = opcode(OP_RETURN, 0);     // return R0

    run();

    ASSERT_EQ(vm.stack[0].as.i32, 30);
    ASSERT_TRUE(vm.call_stack.empty());
}

TEST(VmFunctionTest, DeepRecursion) {
    VMData& vm = initVM();

    // Recursive countdown function
    Function countdown{};
    countdown.entry_point = 5;
    countdown.arity = 1;
    countdown.local_count = 2;

    vm.functions[0] = countdown;
    vm.constants = {int_val(1), int_val(CALL_MAX_SIZE)};

    // Main code - start with 100
    vm.code[0] = opcode(OP_LOAD, 1, 1);  // R1 = MAX
    vm.code[1] = opcode(OP_CALL, 0, 1, 1); // countdown(MAX)
    vm.code[2] = opcode(OP_HALT);

    // Function code
    // if (n <= 1) return 1
    vm.code[5] = opcode(OP_LOAD, 2, 0);  // R2 = 1
    vm.code[6] = opcode(OP_LE, 3, 1, 2); // R3 = (R1 <= 1)
    vm.code[7] = jmpf(3, 3);  // if !R3 jump +3
    vm.code[8] = opcode(OP_LOAD, 0, 0);  // R0 = 1
    vm.code[9] = opcode(OP_RETURN, 0);   // return 1
    // else return countdown(n-1)
    vm.code[10] = opcode(OP_LOAD, 2, 0); // R2 = 1
    vm.code[11] = opcode(OP_SUB, 4, 1, 2); // R4 = n-1
    vm.code[12] = opcode(OP_CALL, 0, 4, 1); // countdown(n-1)
    vm.code[13] = opcode(OP_RETURN, 0);    // return result

    run();

    ASSERT_EQ(vm.stack[0].as.i32, 1);
    ASSERT_TRUE(vm.call_stack.empty());
}
// Test for float arithmetic operations
TEST(VmArithmeticTest, FloatOperations) {
    VMData& vm = initVM();

    vm.constants = {float_val(3.5f), float_val(1.5f)};

    vm.code[0] = opcode(OP_LOAD, 1, 0);  // R1 = 3.5
    vm.code[1] = opcode(OP_LOAD, 2, 1);  // R2 = 1.5
    vm.code[2] = opcode(OP_ADD, 3, 1, 2);  // R3 = R1 + R2 (5.0)
    vm.code[3] = opcode(OP_SUB, 4, 1, 2);  // R4 = R1 - R2 (2.0)
    vm.code[4] = opcode(OP_MUL, 5, 1, 2);  // R5 = R1 * R2 (5.25)
    vm.code[5] = opcode(OP_DIV, 6, 1, 2);  // R6 = R1 / R2 (~2.333)
    vm.code[6] = opcode(OP_HALT);

    run();

    EXPECT_FLOAT_EQ(vm.stack[vm.fp + 3].as.f32, 5.0f);
    EXPECT_FLOAT_EQ(vm.stack[vm.fp + 4].as.f32, 2.0f);
    EXPECT_FLOAT_EQ(vm.stack[vm.fp + 5].as.f32, 5.25f);
    EXPECT_FLOAT_EQ(vm.stack[vm.fp + 6].as.f32, 3.5f / 1.5f);
}

// Test for mixed int/float arithmetic
TEST(VmArithmeticTest, MixedTypeArithmetic) {
    VMData& vm = initVM();

    vm.constants = {int_val(5), float_val(2.5f)};

    vm.code[0] = opcode(OP_LOAD, 1, 0);  // R1 = 5 (int)
    vm.code[1] = opcode(OP_LOAD, 2, 1);  // R2 = 2.5 (float)
    vm.code[2] = opcode(OP_ADD, 3, 1, 2);  // R3 = R1 + R2 (7.5)
    vm.code[3] = opcode(OP_HALT);

    run();

    EXPECT_EQ(vm.stack[vm.fp + 3].type, ValueType::Float);
    EXPECT_FLOAT_EQ(vm.stack[vm.fp + 3].as.f32, 7.5f);
}

// Test for division by zero
TEST(VmArithmeticTest, DivisionByZero) {
    VMData& vm = initVM();

    vm.constants = {int_val(10), int_val(0)};

    vm.code[0] = opcode(OP_LOAD, 1, 0);  // R1 = 10
    vm.code[1] = opcode(OP_LOAD, 2, 1);  // R2 = 0
    vm.code[2] = opcode(OP_DIV, 3, 1, 2);  // Should throw

    EXPECT_THROW(run(), std::runtime_error);
}

// Test for char operations
TEST(VmOperationsTest, CharOperations) {
    VMData& vm = initVM();

    vm.constants = {char_val('a'), char_val('b')};

    vm.code[0] = opcode(OP_LOAD, 1, 0);  // R1 = 'a'
    vm.code[1] = opcode(OP_LOAD, 2, 1);  // R2 = 'b'
    vm.code[2] = opcode(OP_EQ, 3, 1, 2);  // R3 = ('a' == 'b') -> 0
    vm.code[3] = opcode(OP_LT, 4, 1, 2);  // R4 = ('a' < 'b') -> 1
    vm.code[4] = opcode(OP_HALT);

    run();

    EXPECT_EQ(vm.stack[vm.fp + 3].as.i32, 0);
    EXPECT_EQ(vm.stack[vm.fp + 4].as.i32, 1);
}

// Test for object field access edge cases
TEST(VmObjectTest, ObjectFieldEdgeCases) {
    VMData& vm = initVM();

    // Define object with 2 fields
    auto obj = ObjClass{};
    obj.indexes = {{"x", 0}, {"y", 1}};
    vm.classes = {obj};

    vm.code[0] = opcode(OP_NEWOBJ, 1, 0);  // R1 = new object
    // Try to access out of bounds field
    vm.code[1] = opcode(OP_GETFIELD, 2, 1, 2);  // Should throw

    EXPECT_THROW(run(), std::out_of_range);
}

// Test for native function calls
TEST(VmNativeTest, NativeFunctionCall) {
    VMData& vm = initVM();

    // Define a native function that sets R0 to 42
    vm.natives[0] = [](VMData& vm) {
        Value val;
        val.type = ValueType::Int;
        val.as.i32 = 42;
        vm.stack[vm.fp] = val;
    };

    vm.code[0] = opcode(OP_NATIVE_CALL, 0);
    vm.code[1] = opcode(OP_HALT);

    run();

    EXPECT_EQ(vm.stack[vm.fp].as.i32, 42);
}

// Test for dynamic function invocation
TEST(VmFunctionTest, DynamicInvocation) {
    VMData& vm = initVM();

    // Define a simple function
    Function func{};
    func.entry_point = 5;
    func.arity = 1;
    func.local_count = 2;
    vm.functions[0] = func;

    // Create callable value
    Value callable;
    callable.type = ValueType::Callable;
    callable.as.callable = 0;
    vm.constants = {callable, int_val(10)};

    // Main code
    vm.code[0] = opcode(OP_LOAD, 1, 0);  // R1 = callable
    vm.code[1] = opcode(OP_LOAD, 2, 1);  // R2 = 10 (arg)
    vm.code[2] = opcode(OP_INVOKEDYNAMIС, 1, 2, 1);  // R1(R2)
    vm.code[3] = opcode(OP_HALT);
    // Function just returns the argument
    vm.code[5] = opcode(OP_ADD, 2, 1, 1);  // R2 = R1 + R1
    vm.code[6] = opcode(OP_RETURN, 2);

    run();

    EXPECT_EQ(vm.stack[vm.fp].as.i32, 20);
}

// Test for stack overflow protection
TEST(VmMemoryTest, StackOverflow) {
    VMData& vm = initVM();

    // Recursive function that would overflow the stack
    Function recurse{};
    recurse.entry_point = 5;
    recurse.arity = 0;
    recurse.local_count = 1;
    vm.functions[0] = recurse;

    // Main code
    vm.code[0] = opcode(OP_CALL, 0, 0, 0);  // recurse()
    vm.code[1] = opcode(OP_HALT);
    // Function code - infinite recursion
    vm.code[5] = opcode(OP_CALL, 0, 0, 0);  // recurse()
    vm.code[6] = opcode(OP_RETURNNIL);

    EXPECT_THROW(run(), std::runtime_error);
}

// Test for truthy/falsy values
TEST(VmOperationsTest, TruthyValues) {
    VMData& vm = initVM();

    vm.constants = {
            int_val(0), int_val(1),
            float_val(0.0f), float_val(1.0f),
            char_val('\0'), char_val('a'),
            Value()  // nil
    };

    // Test various values for truthiness
    vm.code[0] = opcode(OP_LOAD, 1, 0);  // R1 = 0 (int)
    vm.code[1] = opcode(OP_LOAD, 2, 1);  // R2 = 1 (int)
    vm.code[2] = opcode(OP_LOAD, 3, 2);  // R3 = 0.0 (float)
    vm.code[3] = opcode(OP_LOAD, 4, 3);  // R4 = 1.0 (float)
    vm.code[4] = opcode(OP_LOAD, 5, 4);  // R5 = '\0' (char)
    vm.code[5] = opcode(OP_LOAD, 6, 5);  // R6 = 'a' (char)
    vm.code[6] = opcode(OP_LOAD, 7, 6);  // R7 = nil

    // Check truthiness
    vm.code[7] = jmpt(1, 100);  // Should not jump (0 is falsy)
    vm.code[8] = jmpt( 2, 1);  // Should jump (1 is truthy)
    vm.code[9] = jmpt( 3, 100);  // Should not jump (0.0 is falsy)
    vm.code[10] = jmpt( 4, 1); // Should jump (1.0 is truthy)
    vm.code[11] = jmpt( 5, 100); // Should not jump ('\0' is falsy)
    vm.code[12] = jmpt( 6, 1); // Should jump ('a' is truthy)
    vm.code[13] = jmpt( 7, 100); // Should not jump (nil is falsy)
    vm.code[14] = opcode(OP_HALT);

    // We can't directly test jumps in this simple test, but at least verify no exceptions
    EXPECT_NO_THROW(run());
}

// Test for invalid opcode
TEST(VmErrorTest, InvalidOpcode) {
    VMData& vm = initVM();

    vm.code[0] = 0xFFFFFFFF;  // Invalid opcode
    vm.code[1] = opcode(OP_HALT);

    EXPECT_THROW(run(), std::runtime_error);
}

// Test for constant pool out of range
TEST(VmErrorTest, ConstantOutOfRange) {
    VMData& vm = initVM();

    vm.code[0] = opcode(OP_LOAD, 1, 100);  // Try to access non-existent constant
    vm.code[1] = opcode(OP_HALT);

    EXPECT_THROW(run(), std::out_of_range);
}

// Test for function call with wrong arity
TEST(VmFunctionTest, WrongArityCall) {
    VMData& vm = initVM();

    // Function expects 2 args
    Function func{};
    func.entry_point = 5;
    func.arity = 2;
    func.local_count = 3;
    vm.functions[0] = func;
    vm.constants = {nil_val()};

    // Call with 1 arg
    vm.code[0] = opcode(OP_LOAD, 1, 0);  // R1 = arg
    vm.code[1] = opcode(OP_CALL, 0, 1, 1);  // Should throw

    vm.code[5] = opcode(OP_RETURNNIL);

    EXPECT_THROW(run(), std::runtime_error);
}
