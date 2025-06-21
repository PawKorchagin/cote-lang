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
    int freed = 0;

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
            auto *block = vm.heap[i].get();
            if (block == nullptr) continue;
            block->unmark();
        }
    }

    int alive_cnt;

    void mark_alive(interpreter::VMData &vm, interpreter::Value &value) {
        if (value.is_array()) {
            const auto size = value.i32;
            const auto ptr = value.object_ptr;
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
            //if is not an array, then it is not a gc object and is WAS NOT ALLOCATED and we shouldn't even consider collecting it
//            value.mark();
            alive_cnt++;
        }
    }

    void mark(interpreter::VMData &vm) {
        alive_cnt = 0;
        log("mark\n");
        // TODO sp
        for (int i = 0; i < vm.fp + 100; ++i) {
            if (vm.stack[i].is_object()) {
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
            const auto *block = vm.heap[i].get();
            if (block == nullptr) continue;
            if (!block->is_marked()) {
                log("heap reset: ");
                log(i);
                log("\n");
                vm.heap[i].reset();
                if (vm.heap[i].use_count() == 0) {
                    freed++;
                }
            }
        }
        log("\n");
    }

    int get_freed_objects() {
        return freed;
    }
} // gc
