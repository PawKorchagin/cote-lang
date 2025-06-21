//
// Created by Георгий on 20.06.2025.
//

#include "gc.h"
#include "vm.h"

#include <cassert>
#include <iostream>
#include<ranges>

namespace gc {
    void reset_alive(const interpreter::VMData &);

    void mark(interpreter::VMData &);

    void sweep(interpreter::VMData &);

    int calls = 0;

    void call(interpreter::VMData &vm) {
        log("CALL GC ");
        log(calls++);
        log("\n");
        reset_alive(vm);
        mark(vm);
        sweep(vm);
    }

    void reset_alive(const interpreter::VMData &vm) {
        log("\nreset alive\n");
        for (int i = 0; i < vm.heap_size; ++i) {
            auto* block = vm.heap[i].get();
            if (block == nullptr) continue;
            block->class_ptr = 0;
        }
    }

    int alive_cnt;

    void mark_alive(interpreter::VMData &vm, interpreter::Value &value) {
        if (value.is_array()) {
            const auto size = value.as.i32;
            const auto ptr = value.as.object_ptr;
            const auto mem_ptr = vm.heap[ptr].get();

            for (auto i = 0; i < size + 1; ++i) {
                log("mark array: ");
                log(i);
                log("\n");
                if ((mem_ptr + i) != nullptr) {
                    mark_alive(vm, *(mem_ptr + i));
                }
            }
        } else {
            log("mark value\n");
            value.class_ptr = 1;
            alive_cnt++;
        }
    }

    void mark(interpreter::VMData &vm) {
        alive_cnt = 0;
        log("mark\n");
        // TODO sp
        for (int i = 0; i < vm.fp + 100; ++i) {
            if (vm.stack[i].type == interpreter::ValueType::Object) {
                mark_alive(vm, vm.stack[i]);
            }
        }
        log("alive cnt: ");
        log(alive_cnt);
        log("\n");
    }

    void sweep(interpreter::VMData &vm) {
        log("sweep\nheap size: ");
        log(vm.heap_size);
        log("\n");
        for (auto i = 0; i < vm.heap_size; ++i) {
            const auto* block = vm.heap[i].get();
            if (block == nullptr) continue;
            if (!block->class_ptr) {
                log("heap reset: ");
                log(i);
                log("\n");
                vm.heap[i].reset();
            }
        }
        log("\n");
    }
} // gc
