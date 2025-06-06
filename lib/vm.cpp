#include "vm.h"

#include <cstdlib>
#include <stdexcept>

namespace interpreter {

VMData& vm_instance() {
    static VMData instance;
    return instance;
}

void run() {
    VMData& vm = vm_instance();

    while (true) {
        uint32_t instr = vm.code[vm.ip++];
        OpCode op      = static_cast<OpCode>(instr >> OPCODE_SHIFT);

        uint8_t a   = (instr >> A_SHIFT) & A_ARG;
        uint8_t b   = (instr >> B_SHIFT) & B_ARG;
        uint8_t c   = instr & C_ARG;
        uint32_t bx = instr & BX_ARG;

        switch (op) {
        case OP_LOAD:
            op_load(vm, a, bx);
            break;
        case OP_MOVE:
            op_move(vm, a, b);
            break;
        case OP_LOADNIL:
            op_loadnil(vm, a);
            break;
        case OP_ADD:
            op_add(vm, a, b, c);
            break;
        case OP_SUB:
            op_sub(vm, a, b, c);
            break;
        case OP_MUL:
            op_mul(vm, a, b, c);
            break;
        case OP_DIV:
            op_div(vm, a, b, c);
            break;
        case OP_MOD:
            op_mod(vm, a, b, c);
            break;
        case OP_NEG:
            op_neg(vm, a, b);
            break;
        case OP_EQ:
            op_eq(vm, a, b, c);
            break;
        case OP_LT:
            op_lt(vm, a, b, c);
            break;
        case OP_LE:
            op_le(vm, a, b, c);
            break;
        case OP_JMP: {
            int32_t sbx = static_cast<int32_t>(instr & BX_ARG) - J_ZERO;
            op_jmp(vm, sbx);
            break;
        }
        case OP_JMPT: {
            int32_t sbx = static_cast<int32_t>(instr & BX_ARG) - J_ZERO;
            op_jmpt(vm, a, sbx);
            break;
        }
        case OP_JMPF: {
            int32_t sbx = static_cast<int32_t>(instr & BX_ARG) - J_ZERO;
            op_jmpf(vm, a, sbx);
            break;
        }
        case OP_CALL:
            op_call(vm, a, b);
            break;
        case OP_RETURN:
            op_return(vm, a);
            break;
        case OP_NEWOBJ:
            op_newobj(vm, a, bx);
            break;
        case OP_GETFIELD:
            op_getfield(vm, a, b, c);
            break;
        case OP_SETFIELD:
            op_setfield(vm, a, b, c);
            break;
        case OP_HALT:
            op_halt(vm);
            return;
        default:
            throw std::runtime_error("Unknown opcode");
        }
    }
}

void op_load(VMData& vm, uint8_t reg, uint32_t const_idx) {
    if (const_idx >= vm.constants.size()) {
        throw std::out_of_range("Constant index out of range");
    }
    vm.sp = std::max(static_cast<uint32_t>(reg), vm.sp);
    vm.stack[vm.fp + reg] = vm.constants[const_idx];
}

void op_move(VMData& vm, uint8_t dst, uint8_t src) {
    vm.stack[vm.fp + dst] = vm.stack[vm.fp + src];
}

void op_loadnil(VMData& vm, uint8_t reg) {
    vm.stack[vm.fp + reg] = Value{};
}

void op_add(VMData& vm, uint8_t dst, uint8_t src1, uint8_t src2) {
    Value& v1             = vm.stack[vm.fp + src1];
    Value& v2             = vm.stack[vm.fp + src2];
    vm.stack[vm.fp + dst] = add_values(v1, v2);
}

void op_sub(VMData& vm, uint8_t dst, uint8_t src1, uint8_t src2) {
    Value& v1             = vm.stack[vm.fp + src1];
    Value& v2             = vm.stack[vm.fp + src2];
    vm.stack[vm.fp + dst] = sub_values(v1, v2);
}

void op_mul(VMData& vm, uint8_t dst, uint8_t src1, uint8_t src2) {
    Value& v1             = vm.stack[vm.fp + src1];
    Value& v2             = vm.stack[vm.fp + src2];
    vm.stack[vm.fp + dst] = mul_values(v1, v2);
}

void op_div(VMData& vm, uint8_t dst, uint8_t src1, uint8_t src2) {
    Value& v1             = vm.stack[vm.fp + src1];
    Value& v2             = vm.stack[vm.fp + src2];
    vm.stack[vm.fp + dst] = div_values(v1, v2);
}

void op_mod(VMData& vm, uint8_t dst, uint8_t src1, uint8_t src2) {
    Value& v1 = vm.stack[vm.fp + src1];
    Value& v2 = vm.stack[vm.fp + src2];

    if (v1.is_int() && v2.is_int()) {
        if (v2.as.i32 == 0)
            throw std::runtime_error("Division by zero");

        Value res;
        res.type = ValueType::Int;
        res.as.i32 = v1.as.i32 % v2.as.i32;
        vm.stack[vm.fp + dst] = res;
    } else {
        throw std::runtime_error("Modulo requires integer operands");
    }
}

void op_neg(VMData& vm, uint8_t dst, uint8_t src) {
    Value& v = vm.stack[vm.fp + src];
    Value res;

    if (v.is_int()) {
        res.type = ValueType::Int;
        res.as.i32 = -v.as.i32;
    } else if (v.is_float()) {
        res.type = ValueType::Float;
        res.as.f32 = -v.as.f32;
    } else {
        throw std::runtime_error("Cannot negate non-numeric value");
    }
    vm.stack[vm.fp + dst] = res;
}

void op_eq(VMData& vm, const uint8_t dst, const uint8_t src1, const uint8_t src2) {
    auto& [type1, data1] = vm.stack[vm.fp + src1];
    auto& [type2, data2] = vm.stack[vm.fp + src2];

    Value res;
    res.type = ValueType::Int;
    res.as.i32 = 0;

    if (type1 == type2) {
        switch (type1) {
        case ValueType::Int:
            res.as.i32 = data1.i32 == data2.i32;
            break;
        case ValueType::Float:
            res.as.i32 = data1.f32 == data2.f32;
            break;
        case ValueType::Char:
            res.as.i32 = data1.c == data2.c;
            break;
        case ValueType::Object:
            res.as.i32 = data1.object_ptr == data2.object_ptr;
            break;
        case ValueType::Nil:
            res.as.i32 = 1;
            break;
        default:
            res.as.i32 = 0;
        }
    }
    vm.stack[vm.fp + dst] = res;
}

void op_lt(VMData& vm, uint8_t dst, uint8_t src1, uint8_t src2) {
    const Value& v1 = vm.stack[vm.fp + src1];
    const Value& v2 = vm.stack[vm.fp + src2];
    Value res;
    res.type = ValueType::Int;

    if (v1.is_int() && v2.is_int())
        res.as.i32 = v1.as.i32 < v2.as.i32;
    else if (v1.is_float() && v2.is_float())
        res.as.i32 = v1.as.f32 < v2.as.f32;
    else if (v1.is_char() && v2.is_char())
        res.as.i32 = v1.as.c < v2.as.c;
    else
        throw std::runtime_error("Comparison requires compatible types");

    vm.stack[vm.fp + dst] = res;
}

void op_le(VMData& vm, uint8_t dst, uint8_t src1, uint8_t src2) {
    const Value& v1 = vm.stack[vm.fp + src1];
    const Value& v2 = vm.stack[vm.fp + src2];
    Value res;
    res.type = ValueType::Int;

    if (v1.is_int() && v2.is_int())
        res.as.i32 = v1.as.i32 <= v2.as.i32;
    else if (v1.is_float() && v2.is_float())
        res.as.i32 = v1.as.f32 <= v2.as.f32;
    else if (v1.is_char() && v2.is_char())
        res.as.i32 = v1.as.c <= v2.as.c;
    else
        throw std::runtime_error("Comparison requires compatible types");

    vm.stack[vm.fp + dst] = res;
}

void op_jmp(VMData& vm, int32_t offset) {
    vm.ip += offset;
}

void op_jmpt(VMData& vm, uint8_t cond, int32_t offset) {
    if (is_truthy(vm.stack[vm.fp + cond])) {
        vm.ip += offset;
    }
}

void op_jmpf(VMData& vm, uint8_t cond, int32_t offset) {
    if (!is_truthy(vm.stack[vm.fp + cond])) {
        vm.ip += offset;
    }
}

void op_call(VMData& vm, uint8_t func_idx, uint8_t arg_count) {
    if (func_idx >= FUNCTIONS_MAX) {
        throw std::out_of_range("Function index out of range");
    }

    Function& func = vm.functions[func_idx];
    if (arg_count != func.arity) {
        throw std::runtime_error("Argument count mismatch");
    }

    vm.call_stack.push(CallFrame{vm.ip, vm.fp});

    vm.fp = vm.sp;
    vm.sp += func.local_count;

    vm.ip = func.entry_point;
}

void op_return(VMData& vm, uint8_t result_reg) {
    if (vm.call_stack.empty()) {
        op_halt(vm);
        return;
    }

    // Save return value
    Value result = vm.stack[vm.fp + result_reg];

    // Restore previous frame
    CallFrame frame = vm.call_stack.top();
    vm.call_stack.pop();

    vm.sp = vm.fp;            // Pop locals
    vm.fp = frame.base_ptr;   // Restore frame pointer
    vm.ip = frame.return_ip;  // Restore instruction pointer

    // Push result to caller's stack
    vm.stack[vm.fp] = result;
}

// TAG: GC maybe want to do smth here
void op_newobj(VMData& vm, uint8_t dst, uint32_t context_idx) {
    if (context_idx >= vm.contexts.size()) {
        throw std::out_of_range("context index out of range");
    }
    if (vm.heap_size >= HEAP_MAX_SIZE) {
        //TAG: GC
    }

    const ObjectContext field_count = vm.contexts[context_idx];
    Object* obj = new Object{context_idx, std::vector<Value>(field_count)};
    vm.heap[vm.heap_size] = obj;

    Value newobj;
    newobj.type = ValueType::Object;
    newobj.as.object_ptr = vm.heap_size;
    vm.stack[vm.fp + dst] = newobj;

    vm.heap_size++;
}

void op_getfield(VMData& vm, uint8_t dst, uint8_t obj, uint8_t field_idx) {
    Value& obj_val  = vm.stack[vm.fp + obj];
    Object* obj_ptr = vm.heap[obj_val.as.object_ptr];
    if (field_idx >= obj_ptr->fields.size()) {
        throw std::out_of_range("No such field");
    }

    vm.stack[vm.fp + dst] = obj_ptr->fields[field_idx];
}

void op_setfield(VMData& vm, uint8_t obj, uint8_t field_idx, uint8_t src) {
    Value& obj_val  = vm.stack[vm.fp + obj];
    Object* obj_ptr = vm.heap[obj_val.as.object_ptr];
    if (field_idx >= obj_ptr->fields.size()) {
        throw std::out_of_range("No such field");
    }

    obj_ptr->fields[field_idx] = vm.stack[vm.fp + src];
}

void op_halt(VMData& vm) {
    vm.ip = CODE_MAX_SIZE - 1;  // Stop execution
}

Value add_values(const Value& a, const Value& b) {
    Value res;

    if (a.is_int() && b.is_int()) {
        res.type   = ValueType::Int;
        res.as.i32 = a.as.i32 + b.as.i32;
    } else {
        res.type   = ValueType::Float;
        float fa   = a.is_int() ? static_cast<float>(a.as.i32) : a.as.f32;
        float fb   = b.is_int() ? static_cast<float>(b.as.i32) : b.as.f32;
        res.as.f32 = fa + fb;
    }

    return res;
}

Value sub_values(const Value& a, const Value& b) {
    Value res;

    if (a.is_int() && b.is_int()) {
        res.type   = ValueType::Int;
        res.as.i32 = a.as.i32 - b.as.i32;
    } else {
        res.type   = ValueType::Float;
        float fa   = a.is_int() ? static_cast<float>(a.as.i32) : a.as.f32;
        float fb   = b.is_int() ? static_cast<float>(b.as.i32) : b.as.f32;
        res.as.f32 = fa - fb;
    }

    return res;
}

Value mul_values(const Value& a, const Value& b) {
    Value res;

    if (a.is_int() && b.is_int()) {
        res.type   = ValueType::Int;
        res.as.i32 = a.as.i32 * b.as.i32;
    } else {
        res.type   = ValueType::Float;
        float fa   = a.is_int() ? static_cast<float>(a.as.i32) : a.as.f32;
        float fb   = b.is_int() ? static_cast<float>(b.as.i32) : b.as.f32;
        res.as.f32 = fa * fb;
    }

    return res;
}

Value div_values(const Value& a, const Value& b) {
    if (b.is_int() && b.as.i32 == 0)
        throw std::runtime_error("Division by zero");
    if (b.is_float() && b.as.f32 == 0.0f)
        throw std::runtime_error("Division by zero");

    Value res;
    if (a.is_int() && b.is_int()) {
        res.type   = ValueType::Int;
        res.as.i32 = a.as.i32 / b.as.i32;
    } else {
        res.type   = ValueType::Float;
        float fa   = a.is_int() ? static_cast<float>(a.as.i32) : a.as.f32;
        float fb   = b.is_int() ? static_cast<float>(b.as.i32) : b.as.f32;
        res.as.f32 = fa / fb;
    }

    return res;
}

bool is_truthy(const Value& val) {
    if (val.is_nil())
        return false;
    if (val.is_int())
        return val.as.i32 != 0;
    if (val.is_float())
        return val.as.f32 != 0.0f;
    if (val.is_char())
        return val.as.c != '\0';
    return true;  // Objects are always truthy
}

void init_vm(std::istream& in) {
    // Initialization logic would go here
    // Load bytecode, constants, contextes, etc.
}

}  // namespace interpreter
