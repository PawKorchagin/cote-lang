//
// Created by Георгий on 22.06.2025.
//

#ifndef HEAP_H
#define HEAP_H

#include <vector>

#include "vm.h"

#define GC_TEST

namespace heap {
#ifdef GC_TEST
    inline std::vector<std::shared_ptr<interpreter::Value[]>> heap;
#else
    inline std::vector<interpreter::Value*> heap;
#endif


    // inline auto &get_heap() {
    //     return heap;
    // }
    //
    inline auto& get_heap(const size_t addr, const bool nullable = false) {
        if (addr >= heap.size()) {
            throw std::out_of_range("out of heap range while getting heap by addr");
        }

        if (!nullable) {
#ifdef GC_TEST
            const auto* obj = heap[addr].get();
#else
            const auto* obj = heap[addr];
#endif

            if (obj == nullptr) {
                throw std::runtime_error("get non nullable heap addr, but got nullptr obj");
            }
        }

        return heap[addr];
    }

    size_t get_size();

    inline auto is_empty() {
        return heap.empty();
    }
} // heap

#endif //HEAP_H
