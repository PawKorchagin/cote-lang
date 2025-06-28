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

void cote_print_(interpreter::VMData &vm, int reg, int cnt) {
    int off = vm.fp + reg;
    for (int i = 0; i < cnt; ++i) {
        auto &cur = vm.stack[off + i];
        if (cur.is_callable()) std::cout << "callable " << cur.i32;
        else if (cur.is_int()) std::cout << cur.i32 << ' ';
        else if (cur.is_float()) std::cout << cur.f32;
        else if (cur.is_nil()) std::cout << "nil";
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

// get number of objects in memory at the moment
void GET_OBJECTS(interpreter::VMData &vm, int reg, int cnt) {
    if (cnt) {
        throw std::runtime_error("no args expected");
    }

    vm.stack[vm.fp + reg].set_int(
        static_cast<int>(
            vm.gc.young_roots.size()
            + vm.gc.old_roots.size()
            + vm.gc.large_roots.size()
        ));
}

void GET_YOUNG(interpreter::VMData &vm, int reg, int cnt) {
    if (cnt) {
        throw std::runtime_error("no args expected");
    }

    vm.stack[vm.fp + reg].set_int(vm.gc.young_roots.size());
}

void GET_LARGE(interpreter::VMData &vm, int reg, int cnt) {
    if (cnt) {
        throw std::runtime_error("no args expected");
    }

    vm.stack[vm.fp + reg].set_int(
        static_cast<int>(
            0
            + vm.gc.large_roots.size()
        ));
}

void GET_OLD(interpreter::VMData &vm, int reg, int cnt) {
    if (cnt) {
        throw std::runtime_error("no args expected");
    }

    vm.stack[vm.fp + reg].set_int(
        static_cast<int>(
            0
            + vm.gc.old_roots.size()
        ));
}

void ASSERT(interpreter::VMData& vm, int reg, int cnt) {
    for (int i = 0; i < cnt; ++i) {
        assert(vm.stack[vm.fp + reg + i].i32 == 1 && "FALSE CONDITION");
    }
}

// force gc call
void GC_CALL(interpreter::VMData &vm, int, int cnt) {
    if (cnt) {
        throw std::runtime_error("no args expected");
    }
    vm.gc.call();
}


void cote_stdlib::initStdlib(interpreter::VMData &data, parser::VarManager &vars) {
    data.natives[vars.add_native("print")] = cote_print;
    data.natives[vars.add_native("str")] = cote_str;
    data.natives[vars.add_native("println")] = cote_println;
    data.natives[vars.add_native("len")] = cote_len;
    data.natives[vars.add_native("rand")] = cote_rand;
    data.natives[vars.add_native("throw")] = cote_throw;
    // GC MONITOR NATIVES:
    data.natives[vars.add_native("GET_OBJECTS")] = GET_OBJECTS;
    data.natives[vars.add_native("GET_YOUNG")] = GET_YOUNG;
    data.natives[vars.add_native("GET_LARGE")] = GET_LARGE;
    data.natives[vars.add_native("GET_OLD")] = GET_OLD;
    data.natives[vars.add_native("GC_CALL")] = GC_CALL;

    // just prints space after ints
    data.natives[vars.add_native("print_")] = cote_print_;

    // ASSERT
    data.natives[vars.add_native("ASSERT")] = ASSERT;
}

