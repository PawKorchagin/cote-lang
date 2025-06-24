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
    // auto &obj = vm.heap[cur.object_ptr];
#ifdef GC_TEST
    const auto* obj = heap::get_heap(cur.object_ptr).get();
#else
    const auto* obj = cur.object_ptr;
#endif
    vm.stack[vm.fp + reg] = obj[0];
}

void cote_print(interpreter::VMData &vm, int reg, int cnt) {
    int off = vm.fp + reg;
    for (int i = 0; i < cnt; ++i) {
        auto &cur = vm.stack[off + i];
        if (cur.is_callable()) std::cout << "callable " << cur.i32;
        else if (cur.is_int()) std::cout << cur.i32;
        else if (cur.is_float()) std::cout << cur.f32;
        else if (cur.is_nil()) std::cout << "nil";
        else throw std::runtime_error("todo");
    }
}


void cote_println(interpreter::VMData &vm, int reg, int cnt) {
    int off = vm.fp + reg;
    for (int i = 0; i < cnt; ++i) {
        auto &cur = vm.stack[off + i];
        if (cur.is_callable()) std::cout << "callable " << cur.i32 << ' ';
        else if (cur.is_int()) std::cout << cur.i32 << ' ';
        else if (cur.is_float()) std::cout << cur.f32 << ' ';
        else if (cur.is_nil()) std::cout << "nil" << ' ';
        else throw std::runtime_error("todo or println dont know what is it");
    }
    std::cout << std::endl;
}

void cote_stdlib::initStdlib(interpreter::VMData &data, parser::VarManager &vars) {
    data.natives[vars.add_native("print")] = cote_print;
    data.natives[vars.add_native("str")] = cote_str;
    data.natives[vars.add_native("println")] = cote_println;
    data.natives[vars.add_native("len")] = cote_len;
}