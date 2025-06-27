//
// Created by motya on 19.06.2025.
//
#include "lang_stdlib.h"

#include "heap.h"

void cote_str(interpreter::VMData &vm, int reg, int cnt) {
    throw std::runtime_error("todo");
}

void cote_len(interpreter::VMData &vm, int reg, int cnt) {
    if (cnt != 1) throw std::runtime_error("expected only one arg: array");
    auto &cur = vm.stack[vm.fp + reg];
    if (!cur.is_array()) throw std::runtime_error("expected only one arg: array");
    auto *obj = heap::mem.at(cur.object_ptr);
    assert(obj);
    vm.stack[vm.fp + reg].set_int(static_cast<int>(obj->get_len()));
}

void cote_print(interpreter::VMData &vm, int reg, int cnt) {
    int off = vm.fp + reg;
    for (int i = 0; i < cnt; ++i) {
        auto &cur = vm.stack[off + i];
        if (cur.is_callable()) std::cout << "callable " << cur.i32;
        else if (cur.is_int()) std::cout << cur.i32 << ' ';
        else if (cur.is_float()) std::cout << cur.f32 << ' ';
        else if (cur.is_nil()) std::cout << "nil ";
        else throw std::runtime_error("todo");
    }
}


void cote_println(interpreter::VMData &vm, int reg, int cnt) {
    cote_print(vm, reg, cnt);
    std::cout << std::endl;
}

void cote_rand(interpreter::VMData &vm, int reg, int cnt) {
    if (cnt != 0) throw std::runtime_error("rand requires no arguments");
    vm.stack[vm.fp + reg].set_int(rand());
}
void cote_throw(interpreter::VMData &vm, int reg, int cnt) {
    if (cnt != 0) throw std::runtime_error("throw requires no arguments");
    throw std::runtime_error("cote throw was called");
}
// void GC_DROP_MINOR(interpreter::VMDaVta& vm, int reg, int cnt) {
// vm.gc.rese
// }

void cote_stdlib::initStdlib(interpreter::VMData &data, parser::VarManager &vars) {
    data.natives[vars.add_native("print")] = cote_print;
    data.natives[vars.add_native("str")] = cote_str;
    data.natives[vars.add_native("println")] = cote_println;
    data.natives[vars.add_native("len")] = cote_len;
    data.natives[vars.add_native("rand")] = cote_rand;
    data.natives[vars.add_native("throw")] = cote_throw;
    // data.natives[vars.add_native("GC_DROP_MINOR")] = GC_DROP_MINOR;
}