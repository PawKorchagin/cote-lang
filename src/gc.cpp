//
// Created by Георгий on 20.06.2025.
//

#include "gc.h"
#include "vm.h"

#include <cassert>
#include <iostream>
#include <ranges>
#include <algorithm>

#include "heap.h"

// namespace gc_detail {
//     inline bool all_non_nullptr() {
//         return std::ranges::all_of(
//             heap::heap,
//             [](const auto &ptr) {
//                 return ptr != nullptr;
//             });
//     }
//
//     inline bool all_non_nullptr(const std::vector<
// #ifdef GC_TEST
//             std::shared_ptr<interpreter::Value[]>
// #else
//         interpreter::Value*
// #endif
//         >::iterator &end
//     ) {
//         return std::ranges::all_of(
//             std::ranges::begin(heap::heap),
//             end,
//             [](const auto &ptr) {
//                 return ptr != nullptr;
//             });
//     }
//
//     inline bool all_nullptr(const std::vector<
// #ifdef GC_TEST
//         std::shared_ptr<interpreter::Value[]>
// #else
//             interpreter::Value*
// #endif
//     >::iterator &begin) {
//         return std::ranges::all_of(
//             begin,
//             std::ranges::end(heap::heap),
//             [](const auto &ptr) {
//                 return ptr == nullptr;
//             });
//     }
// }
//
// namespace gc {
//     void reset_alive();
//
//     void mark(interpreter::VMData &);
//
//     void sweep(interpreter::VMData &);
//
//     int calls = 0;
//     int freed = 0;
//     bool verbosity = false;
//
//     void gc::call(interpreter::VMData &vm, const bool verbose) {
//         verbosity = verbose;
//         calls++;
//         reset_alive();
//         mark(vm);
//         sweep();
//         if (verbose) {
//             std::cerr << "Call: " << calls << ", Freed: " << get_freed_objects() << " Heap: " << heap::get_size()
//                     << std::endl;
//         }
//     }
//
//     void reset_alive() {
//         // PRE: no nullptr in heap
//         assert(gc_detail::all_non_nullptr());
//         for (
// #ifdef GC_TEST
//             const std::shared_ptr<interpreter::Value[]>
// #else
//             interpreter::Value*
// #endif
//             &ptr: heap::heap) {
//             ptr // NOLINT(*-redundant-smartptr-get)
// #ifdef GC_TEST
//                     .get()
// #endif
//                     ->unmark();
//         }
//         // POST: no nullptr in heap
//         assert(gc_detail::all_non_nullptr());
//     }
//
//     int alive_cnt;
//
//     void mark_alive(const interpreter::Value &value) {
//         assert(value.is_object());
// #ifdef GC_TEST
//         heap::get_heap(value.object_ptr).get()->mark();
// #else
//         heap::get_heap(value.object_ptr)->mark();
// #endif
//     }
//
//     void mark(interpreter::VMData &vm) {
//         if (heap::is_empty()) {
//             return;
//         }
//         alive_cnt = 0;
//         for (int i = 0; i < vm.get_sp(); ++i) {
//             if (vm.stack[i].is_object()) {
//                 mark_alive(vm.stack[i]);
//             }
//         }
//     }
//
//     void sweep(interpreter::VMData& vm) {
//         if (heap::is_empty()) {
//             return;
//         }
// #ifdef GC_TEST
//         // PRE: no nullptr in heap, heap::get_size() > 0
//         assert(gc_detail::all_non_nullptr());
//         int deleted = 0;
//         for (size_t i = 0; i < heap::get_size(); ++i) {
//             // INV: if heap[i] == nullptr, forall j > i heap[i] == nullptr
//             if (heap::heap[i] == nullptr) {
//                 assert(gc_detail::all_nullptr(heap::heap.begin() + i));
//                 break;
//             }
//             if (auto &ptr = heap::get_heap(i); !ptr.get()->is_marked()) {
//                 ptr.reset();
//
//                 const size_t j = heap::get_size() - deleted - 1;
//                 deleted++;
//                 if (i == j) {
//                     // no need to swap i, j
//                     break;
//                 }
//
//                 heap::get_heap(j).swap(ptr);
//
//                 if (ptr.use_count() == 0) {
//                     freed++;
//                 }
//             }
//         }
//         // PRE: all non nullptr
//         // or exists i:
//         // i = heap::get_size() - deleted >= 0
//         // i <=> first nullptr index
//         const int first_nullptr_index = static_cast<int>(heap::get_size()) - deleted;
//         assert(first_nullptr_index >= 0);
//         if (verbosity) {
//             std::cerr << "Heap size: " << heap::get_size() << std::endl << "Deleted: " << deleted << std::endl <<
//                     "first nullptr index: " << first_nullptr_index << std::endl;
//         }
//         // forall j < i: heap[j] != nullptr,
//         assert(gc_detail::all_non_nullptr(heap::heap.begin() + first_nullptr_index));
//         // forall k >=i heap[j] == nullptr
//         assert(gc_detail::all_nullptr(heap::heap.begin() + first_nullptr_index));
//         while (!heap::heap.empty() && heap::heap.back() == nullptr) {
//             heap::heap.pop_back();
//         }
//         // POST: no nullptr in heap
//         assert(gc_detail::all_non_nullptr());
// #else
// #endif
//     }
//
//     int gc::get_freed_objects() {
//         return freed;
//     }
//
//     int gc::get_calls() {
//         return calls;
//     }
// } // gc
