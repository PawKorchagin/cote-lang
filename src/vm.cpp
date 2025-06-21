#include "vm.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "gc.h"

namespace {
    interpreter::VMData vm_instance_;
}
namespace interpreter {
    VMData &vm_instance() {
        return vm_instance_;
    }

    void run() {
        VMData &vm = vm_instance();

        int T = 0;

        while (true) {
            T++;

            if (T % 10 == 0) {
                // log("CALL GC");
                gc::call(vm);
            }

            uint32_t instr = vm.code[vm.ip++];
            OpCode op = static_cast<OpCode>(instr >> OPCODE_SHIFT);

            uint8_t a = (instr >> A_SHIFT) & A_ARG;
            uint8_t b = (instr >> B_SHIFT) & B_ARG;
            uint8_t c = instr & C_ARG;
            uint32_t bx = instr & BX_ARG;

            switch (op) {
                case OP_LOADINT:
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
                case OP_NEQ:
                    op_neq(vm, a, b, c);
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
                    op_call(vm, a, b, c);
                    break;
                case OP_NATIVE_CALL:
                    op_native_call(vm, a, b, c);
                    break;
                case OP_INVOKEDYNAMIC:
                    op_invokedyn(vm, a, b, c);
                    break;
                case OP_RETURN:
                    op_return(vm, a);
                    break;
                case OP_RETURNNIL:
                    op_returnnil(vm);
                    break;
                case OP_HALT:
                    op_halt(vm);
                    return;
                case OP_LOADFLOAT:
                    op_loadfloat(vm, a, bx);
                    break;
                case OP_LOADFUNC:
                    op_loadfunc(vm, a, bx);
                    break;
                case OP_ALLOC:
                    op_alloc(vm, a, b);
                    break;
                case OP_ARRGET:
                    op_arrget(vm, a, b, c);
                    break;
                case OP_ARRSET:
                    op_arrset(vm, a, b, c);
                    break;
                case OP_TAILCALL:
                    // op_tailcall(vm, a, b, c);
                    throw std::runtime_error("Tailcall not implemented");
                default:
                    throw std::runtime_error("Unknown opcode");
            }
        }
    }

    void op_load(VMData &vm, uint8_t reg, uint32_t const_idx) {
        if (const_idx >= vm.constanti.size()) {
            throw std::out_of_range("Constant index out of range");
        }
        vm.stack[vm.fp + reg] = vm.constanti[const_idx];
    }

    void op_move(VMData &vm, uint8_t dst, uint8_t src) {
        vm.stack[vm.fp + dst] = vm.stack[vm.fp + src];
    }

    void op_loadnil(VMData &vm, uint8_t reg) {
        vm.stack[vm.fp + reg] = Value{};
    }

    void op_add(VMData &vm, uint8_t dst, uint8_t src1, uint8_t src2) {
        Value &v1 = vm.stack[vm.fp + src1];
        Value &v2 = vm.stack[vm.fp + src2];
        vm.stack[vm.fp + dst] = add_values(v1, v2);
    }

    void op_sub(VMData &vm, uint8_t dst, uint8_t src1, uint8_t src2) {
        Value &v1 = vm.stack[vm.fp + src1];
        Value &v2 = vm.stack[vm.fp + src2];
        vm.stack[vm.fp + dst] = sub_values(v1, v2);
    }

    void op_mul(VMData &vm, uint8_t dst, uint8_t src1, uint8_t src2) {
        Value &v1 = vm.stack[vm.fp + src1];
        Value &v2 = vm.stack[vm.fp + src2];
        vm.stack[vm.fp + dst] = mul_values(v1, v2);
    }

    void op_div(VMData &vm, uint8_t dst, uint8_t src1, uint8_t src2) {
        Value &v1 = vm.stack[vm.fp + src1];
        Value &v2 = vm.stack[vm.fp + src2];
        vm.stack[vm.fp + dst] = div_values(v1, v2);
    }

    void op_mod(VMData &vm, uint8_t dst, uint8_t src1, uint8_t src2) {
        Value &v1 = vm.stack[vm.fp + src1];
        Value &v2 = vm.stack[vm.fp + src2];

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

    void op_neg(VMData &vm, uint8_t dst, uint8_t src) {
        Value &v = vm.stack[vm.fp + src];
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

    void op_eq(VMData &vm, const uint8_t dst, const uint8_t src1, const uint8_t src2) {
        auto &[type1, ignored, data1] = vm.stack[vm.fp + src1];
        auto &[type2, ignored2, data2] = vm.stack[vm.fp + src2];

        Value res;
        res.type = ValueType::Int;
        res.as.i32 = 0;

        // NOTE: should be the same as: res.as.i32 = (type1 == type2) && (data1.as == data2.as)
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

    void op_neq(VMData &vm, const uint8_t dst, const uint8_t src1, const uint8_t src2) {
        op_eq(vm, dst, src1, src2);
        vm.stack[vm.fp + dst].as.i32 = ~vm.stack[vm.fp + dst].as.i32 & 1;
    }

    void op_lt(VMData &vm, uint8_t dst, uint8_t src1, uint8_t src2) {
        const Value &v1 = vm.stack[vm.fp + src1];
        const Value &v2 = vm.stack[vm.fp + src2];
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

    void op_le(VMData &vm, uint8_t dst, uint8_t src1, uint8_t src2) {
        const Value &v1 = vm.stack[vm.fp + src1];
        const Value &v2 = vm.stack[vm.fp + src2];
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

    void op_jmp(VMData &vm, int32_t offset) {
        vm.ip += offset;
    }

    void op_jmpt(VMData &vm, uint8_t cond, int32_t offset) {
        if (is_truthy(vm.stack[vm.fp + cond])) {
            vm.ip += offset;
        }
    }

    void op_jmpf(VMData &vm, uint8_t cond, int32_t offset) {
        if (!is_truthy(vm.stack[vm.fp + cond])) {
            vm.ip += offset;
        }
    }

    void op_call(VMData &vm, uint8_t func_idx, uint8_t first_arg_ind, uint8_t num_args) {
        if (func_idx >= FUNCTIONS_MAX) {
            throw std::out_of_range("Function index out of range");
        }

        Function &func = vm.functions[func_idx];
        if (num_args != func.arity) {
            throw std::runtime_error("Argument count mismatch");
        }

        if (vm.call_stack.size() >= CALL_MAX_SIZE) {
            throw std::runtime_error("Call stack overflow");
        }
        vm.call_stack.push(CallFrame{vm.ip, vm.fp});

        vm.fp = vm.fp + first_arg_ind;
        vm.ip = func.entry_point;
    }

    void op_return(VMData &vm, uint8_t result_reg) {
        if (vm.call_stack.empty()) {
            op_halt(vm);
            return;
        }

        Value result = vm.stack[vm.fp + result_reg];

        // Restore previous frame
        CallFrame frame = vm.call_stack.top();
        vm.call_stack.pop();

        // result in reg0 of current fp
        vm.stack[vm.fp] = result;

        vm.fp = frame.base_ptr;
        vm.ip = frame.return_ip;
    }

    void op_returnnil(VMData &vm) {
        if (vm.call_stack.empty()) {
            op_halt(vm);
            return;
        }

        // Restore previous frame
        CallFrame frame = vm.call_stack.top();
        vm.call_stack.pop();

        vm.fp = frame.base_ptr;   // Restore frame pointer
        vm.ip = frame.return_ip;  // Restore instruction pointer

        Value res;
        res.type = ValueType::Nil;
        vm.stack[vm.fp] = res;
    }

// TAG: GC maybe want to do smth here
    void op_newobj(VMData &vm, uint8_t dst, uint32_t class_idx) {
        if (class_idx >= vm.classes.size()) {
            throw std::out_of_range("context index out of range");
        }
        if (vm.heap_size >= HEAP_MAX_SIZE) {
            // TAG: GC
        }

        const ObjClass context = vm.classes[class_idx];
        // auto obj = vm.heap[context.indexes.size()].get();
        // vm.heap[vm.heap_size] = obj;

        Value newobj;
        newobj.type = ValueType::Object;
        newobj.as.object_ptr = vm.heap_size;
        newobj.class_ptr = class_idx;
        vm.stack[vm.fp + dst] = newobj;

        vm.heap_size++;
    }

    void op_getfield(VMData &vm, uint8_t dst, uint8_t obj, uint8_t field_idx) {
        throw std::runtime_error("todo");
        //        Value &obj_val = vm.stack[vm.fp + obj];
        //        Object *obj_ptr = vm.heap[obj_val.as.object_ptr];
        //        if (field_idx >= obj_ptr->fields.size()) {
        //            throw std::out_of_range("No such field");
        //        }
        //
        //        vm.stack[vm.fp + dst] = obj_ptr->fields[field_idx];
    }

    void op_setfield(VMData &vm, uint8_t obj, uint8_t field_idx, uint8_t src) {
        throw std::runtime_error("todo");
        //        Value &obj_val = vm.stack[vm.fp + obj];
        //        Object *obj_ptr = vm.heap[obj_val.as.object_ptr];
        //        if (field_idx >= obj_ptr->fields.size()) {
        //            throw std::out_of_range("No such field");
        //        }
        //
        //        obj_ptr->fields[field_idx] = vm.stack[vm.fp + src];
    }

    void op_halt(VMData &vm) {
        vm.ip = CODE_MAX_SIZE - 1;  // Stop execution
    }

    Value add_values(const Value &a, const Value &b) {
        Value res;

        if (a.is_int() && b.is_int()) {
            res.type = ValueType::Int;
            res.as.i32 = a.as.i32 + b.as.i32;
        } else {
            res.type = ValueType::Float;
            float fa = a.is_int() ? static_cast<float>(a.as.i32) : a.as.f32;
            float fb = b.is_int() ? static_cast<float>(b.as.i32) : b.as.f32;
            res.as.f32 = fa + fb;
        }

        return res;
    }

    Value sub_values(const Value &a, const Value &b) {
        Value res;

        if (a.is_int() && b.is_int()) {
            res.type = ValueType::Int;
            res.as.i32 = a.as.i32 - b.as.i32;
        } else {
            res.type = ValueType::Float;
            float fa = a.is_int() ? static_cast<float>(a.as.i32) : a.as.f32;
            float fb = b.is_int() ? static_cast<float>(b.as.i32) : b.as.f32;
            res.as.f32 = fa - fb;
        }

        return res;
    }

    Value mul_values(const Value &a, const Value &b) {
        Value res;

        if (a.is_int() && b.is_int()) {
            res.type = ValueType::Int;
            res.as.i32 = a.as.i32 * b.as.i32;
        } else {
            res.type = ValueType::Float;
            float fa = a.is_int() ? static_cast<float>(a.as.i32) : a.as.f32;
            float fb = b.is_int() ? static_cast<float>(b.as.i32) : b.as.f32;
            res.as.f32 = fa * fb;
        }

        return res;
    }

    Value div_values(const Value &a, const Value &b) {
        if (b.is_int() && b.as.i32 == 0)
            throw std::runtime_error("Division by zero");
        if (b.is_float() && b.as.f32 == 0.0f)
            throw std::runtime_error("Division by zero");

        Value res;
        if (a.is_int() && b.is_int()) {
            res.type = ValueType::Int;
            res.as.i32 = a.as.i32 / b.as.i32;
        } else {
            res.type = ValueType::Float;
            float fa = a.is_int() ? static_cast<float>(a.as.i32) : a.as.f32;
            float fb = b.is_int() ? static_cast<float>(b.as.i32) : b.as.f32;
            res.as.f32 = fa / fb;
        }

        return res;
    }

    void op_native_call(VMData &vm, uint8_t func_idx, int reg, int count) {
        vm.natives[func_idx](vm, reg, count);
    }

    void op_invokedyn(VMData &vm, uint8_t a, uint8_t b, uint8_t c) {
        Value callable = vm.stack[vm.fp + a];
        if (!callable.is_callable()) {
            throw std::runtime_error("No expected callable");
        }

        op_call(vm, callable.as.i32, b, c);
    }

    void op_loadint(VMData &vm, uint8_t reg, uint32_t const_idx) {
        if (const_idx >= vm.constanti.size()) {
            throw std::out_of_range("Integer constant index out of range");
        }
        vm.stack[vm.fp + reg].type = ValueType::Int;
        vm.stack[vm.fp + reg].as.i32 = vm.constanti[const_idx].as.i32;
    }

    void op_loadfloat(VMData &vm, uint8_t reg, uint32_t const_idx) {
        if (const_idx >= vm.constantf.size()) {
            throw std::out_of_range("Float constant index out of range");
        }
        vm.stack[vm.fp + reg].type = ValueType::Float;
        vm.stack[vm.fp + reg].as.f32 = vm.constantf[const_idx].as.f32;
    }

    void op_loadfunc(VMData &vm, uint8_t reg, uint32_t const_idx) {
        if (const_idx >= vm.functions_count) {
            throw std::out_of_range("Function constant index out of range");
        }
        vm.stack[vm.fp + reg].type = ValueType::Callable;
        vm.stack[vm.fp + reg].as.i32 = static_cast<int>(const_idx);
    }

    void op_alloc(VMData &vm, uint8_t dst, uint8_t s) {
        if (vm.heap_size >= HEAP_MAX_SIZE) {
            throw std::runtime_error("Heap overflow");
        }

        const uint32_t size = vm.stack[s].as.i32;
        const std::shared_ptr<Value[]> fields(new Value[size + 1]);
        if (!fields) {
            throw std::runtime_error("Memory allocation failed");
        }

        // Set len field
        fields[0].type = ValueType::Int;
        fields[0].as.i32 = size;

        vm.heap[vm.heap_size] = fields;

        Value obj_val;
        obj_val.type = ValueType::Object;
        obj_val.as.object_ptr = vm.heap_size++;
        obj_val.class_ptr = 0;  // Array class is always at index 0

        vm.stack[vm.fp + dst] = obj_val;
    }

    void op_arrget(VMData &vm, uint8_t dst, uint8_t arr, uint8_t idxc) {
        auto &idx = vm.stack[vm.fp + idxc];
        if (!idx.is_int()) {
            throw std::runtime_error("Invalid array index");
        }
        Value &arr_val = vm.stack[vm.fp + arr];
        if (!arr_val.is_object()) {
            throw std::runtime_error("Expected array object");
        }

        auto &obj = vm.heap[arr_val.as.object_ptr];
        Value len_val = obj[0];
        if (!len_val.is_int()) {
            throw std::runtime_error("Invalid array length");
        }

        int32_t len = len_val.as.i32;
        if (idx.as.i32 >= len) {
            throw std::out_of_range("Array index out of bounds");
        }

        vm.stack[vm.fp + dst] = obj[idx.as.i32 + 1];  // +1 because index 0 is len
    }

    void op_arrset(VMData &vm, uint8_t arr, uint8_t idxc, uint8_t src) {
        Value &arr_val = vm.stack[vm.fp + arr];
        auto &idx = vm.stack[vm.fp + idxc];
        if (!idx.is_int()) {
            throw std::runtime_error("Invalid array index");
        }

        if (!arr_val.is_object()) {
            throw std::runtime_error("Expected array object");
        }

        auto &obj = vm.heap[arr_val.as.object_ptr];
        Value len_val = obj[0];
        if (!len_val.is_int()) {
            throw std::runtime_error("Invalid array length");
        }

        int32_t len = len_val.as.i32;
        if (idx.as.i32 >= len) {
            throw std::out_of_range("Array index out of bounds");
        }

        obj[idx.as.i32 + 1] = vm.stack[vm.fp + src];  // +1 because index 0 is len
    }


    bool is_truthy(const Value &val) {
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

    uint32_t opcode(OpCode code, uint8_t a, uint32_t bx) {
        return (static_cast<int>(code) << OPCODE_SHIFT) | (a << A_SHIFT) | bx;
    }

    uint32_t opcode(OpCode code, uint8_t a, uint8_t b, uint8_t c) {
        return (static_cast<int>(code) << OPCODE_SHIFT) | (a << A_SHIFT) | (b << B_SHIFT) | c;
    }

    uint32_t halt() {
        return (static_cast<int>(OP_HALT) << OPCODE_SHIFT);
    }

    uint32_t jmp(int32_t offset) {
        return (static_cast<int>(OP_JMP) << OPCODE_SHIFT) | (offset + J_ZERO);
    }

    uint32_t jmpt(uint8_t a, int32_t offset) {
        return (static_cast<int>(OP_JMPT) << OPCODE_SHIFT) | (a << A_SHIFT) | (offset + J_ZERO);
    }

    uint32_t jmpf(uint8_t a, int32_t offset) {
        return (static_cast<int>(OP_JMPF) << OPCODE_SHIFT) | (a << A_SHIFT) | (offset + J_ZERO);
    }

    uint32_t move(uint8_t a, uint8_t b) {
        return (static_cast<int>(OP_JMPF) << OPCODE_SHIFT) | (a << A_SHIFT) | (b << B_SHIFT) | 0;
    }

    uint32_t opcode(OpCode code, uint8_t reg_index) {
        return (static_cast<int>(code) << OPCODE_SHIFT) | (reg_index << A_SHIFT);
    }

    uint32_t opcode(OpCode code) {
        return (static_cast<int>(code) << OPCODE_SHIFT);
    }

    void print_opcode(uint32_t instruction) {
        // Extract arguments from instruction
        uint8_t opcode = instruction >> OPCODE_SHIFT;
        uint8_t a = (instruction >> A_SHIFT) & 0xFF;
        uint8_t b = (instruction >> B_SHIFT) & 0xFF;
        uint8_t c = (instruction >> C_SHIFT) & 0xFF;
        uint16_t bx = (instruction >> SBX_SHIFT) & 0xFFFF;
        uint16_t sbx = bx;

        // Print opcode name and arguments in one switch
        switch (opcode) {
            case OP_LOADINT:
                std::cout << "LOAD        R" << (int) a << " = constants[" << bx << "]";
                break;
            case OP_MOVE:
                std::cout << "MOVE        R" << (int) a << " = R" << (int) b;
                break;
            case OP_LOADNIL:
                std::cout << "LOADNIL     R" << (int) a << " = nil";
                break;
            case OP_ADD:
                std::cout << "ADD         R" << (int) a << " = R" << (int) b << " + R" << (int) c;
                break;
            case OP_SUB:
                std::cout << "SUB         R" << (int) a << " = R" << (int) b << " - R" << (int) c;
                break;
            case OP_MUL:
                std::cout << "MUL         R" << (int) a << " = R" << (int) b << " * R" << (int) c;
                break;
            case OP_DIV:
                std::cout << "DIV         R" << (int) a << " = R" << (int) b << " / R" << (int) c;
                break;
            case OP_MOD:
                std::cout << "MOD         R" << (int) a << " = R" << (int) b << " % R" << (int) c;
                break;
            case OP_NEG:
                std::cout << "NEG         R" << (int) a << " = -R" << (int) b;
                break;
            case OP_EQ:
                std::cout << "EQ          R" << (int) a << " = R" << (int) b << " == R" << (int) c;
                break;
            case OP_LT:
                std::cout << "LT          R" << (int) a << " = R" << (int) b << " < R" << (int) c;
                break;
            case OP_LE:
                std::cout << "LE          R" << (int) a << " = R" << (int) b << " <= R" << (int) c;
                break;
            case OP_JMP:
                std::cout << "JMP         ip += " << sbx;
                break;
            case OP_JMPT:
                std::cout << "JMPT        if R" << (int) a << " ip += " << sbx;
                break;
            case OP_JMPF:
                std::cout << "JMPF        if !R" << (int) a << " ip += " << sbx;
                break;
            case OP_CALL:
                std::cout << "CALL        func[" << (int) a << "](args R" << (int) b << "..R" << (int) (b + c - 1)
                          << ")";
                break;
            case OP_RETURN:
                std::cout << "RETURN      return R" << (int) a;
                break;
            case OP_RETURNNIL:
                std::cout << "RETURNNIL   return nil";
                break;
            case OP_HALT:
                std::cout << "HALT        halt";
                break;
            default:
                std::cout << "UNKNOWN     unknown opcode";
                break;
        }
        std::cout << std::endl;
    }

    void init_vm(std::istream &in) {
        // Initialization logic would go here
        // Load bytecode, constants, contextes, etc.
    }

}  // namespace interpreter
