//
// Created by Георгий on 22.06.2025.
//

#ifndef HEAP_H
#define HEAP_H

#include <cassert>
#include <vector>
#include <memory_resource>
#include <cstddef>
#include <iostream>
#include <map>
#include <stack>
#include <unordered_set>
#include <ranges>
#include <algorithm>

#include "value.h"

#define ALL(a) a.begin(), a.end()

namespace heap {
    inline std::map<uint32_t, interpreter::Value *> mem{};

    template<uint16_t YOUNG_THRESHOLD>
    struct GarbageCollector {
        class YoungArena {
            interpreter::Value *arena = nullptr;
            uint16_t used = 0;

            void alloc_buffer() {
                arena = new interpreter::Value[YOUNG_THRESHOLD];
            }

        public:
            YoungArena() {
                alloc_buffer();
            }

            interpreter::Value *allocate(size_t values) {
                assert(used + values < YOUNG_THRESHOLD);
                if (arena == nullptr) {
                    alloc_buffer();
                }
                // failed to alloc arena
                assert(arena != nullptr);
                auto *res = arena + used;
                used += values;

                return res;
            }

            void release() {
                used = 0;
                delete[] arena;
                arena = nullptr;
            }

            void forget() {
                used = 0;
                alloc_buffer();
            }

            [[nodiscard]] uint16_t get_used() const {
                return used;
            }
        };

        size_t allocated_ = 1;

#ifdef DEFAULT_GC_YOUNG_CAPACITY
        size_t MAJOR_THRESHOLD = 10;
        size_t LARGE_THRESHOLD = 10;
#else
        // set your own
        size_t MAJOR_THRESHOLD = 10;
        size_t LARGE_THRESHOLD = 10;
#endif

        std::pmr::unsynchronized_pool_resource old_arena{};

        // Allocators
        using Alloc = std::pmr::polymorphic_allocator<interpreter::Value>;
        YoungArena young_alloc = YoungArena();
        Alloc large_alloc{std::pmr::new_delete_resource()};
        Alloc old_alloc{&old_arena};

        using value_ptr = interpreter::Value *;
        using obj_ptr_type = uint32_t;

        struct YoungRoot {
            value_ptr roots[YOUNG_THRESHOLD]{};
            uint16_t size_ = 0;

            void push_back(value_ptr ptr) {
                roots[size_++] = ptr;
            }

            void clear() {
                size_ = 0;
            }

            value_ptr operator[](const size_t i) const {
                return roots[i];
            }

            uint16_t size() {
                return size_;
            }
        };

        // Roots
        using DynamicRoot = std::vector<value_ptr>; // no way :(
        YoungRoot young_roots = YoungRoot();
        DynamicRoot large_roots;
        // std::unordered_set<uint32_t>
        // DynamicRoot old_roots;
        std::vector<obj_ptr_type> old_roots;

        interpreter::Value *stack_;
        std::stack<interpreter::CallFrame> *call_stack_;
        uint32_t *fp_;

        void init(interpreter::Value *stack, std::stack<interpreter::CallFrame> *call_stack, uint32_t *fp) {
            stack_ = stack;
            call_stack_ = call_stack;
            fp_ = fp;
        }

        [[nodiscard]] uint32_t get_sp() const {
            if (call_stack_ == nullptr || call_stack_->empty()) {
                return fp_ ? *fp_ : 0;
            }

            return (fp_ ? *fp_ : 0) + call_stack_->top().cur_func->max_stack;
        }

        // reserve len + 1 objects to young arena
        value_ptr alloc_young(const std::size_t len) {
            value_ptr ptr = young_alloc.allocate(len + 1);
            assert(ptr);
            ptr->object_ptr = allocated_;
            mem.insert({allocated_, ptr});
            allocated_++;
            ptr->set_array(len, ptr);
            young_roots.push_back(ptr);
            return ptr;
        }

        // reserve huge array len + 1
        value_ptr alloc_large(const std::size_t len) {
            value_ptr ptr = large_alloc.allocate(len + 1);
            ptr->object_ptr = allocated_;
            mem.insert({allocated_, ptr});
            allocated_++;
            ptr->set_array(len, ptr);
            large_roots.push_back(ptr);
            return ptr;
        }

        void reset_young() {
            young_roots.clear();
            young_alloc.release();
        }

        void mark(value_ptr ptr) const {
            if (ptr->is_marked()) return;

            uint32_t len = ptr->get_len();
            ptr->mark();
            for (int i = 1; i < len + 1; ++i) {
                auto *neigh = ptr + i;
                if (neigh->is_array()) {
                    auto *obj_neigh = mem.at(neigh->object_ptr);
                    assert(obj_neigh != ptr + i);
                    mark(obj_neigh);
                }
            }
        }

        template<class T>
        void unmark(typename std::vector<T *>::iterator begin, typename std::vector<T *>::iterator end) {
            std::for_each(begin, end, [](auto ptr) { ptr->unmark(); });
        }

        void mark() const {
            assert(stack_ != nullptr || get_sp() == 0);
            try {
                for (int i = 0; i < get_sp(); ++i) {
                    if (stack_[i].is_array()) {
                        auto *ptr = mem.at(stack_[i].object_ptr);
                        ptr->unmark();
                        mark(ptr);
                    }
                }
            } catch (std::runtime_error &e) {
                std::cerr << e.what() << std::endl;
                exit(1);
            }
        }

        std::unordered_set<uint32_t> vis;

        void rebind_mem(value_ptr ptr) {
            if (vis.contains(ptr->object_ptr)) return;
            assert(ptr->is_array());
            vis.insert(ptr->object_ptr);
            value_ptr old = old_alloc.allocate(ptr->get_len() + 1);
            *old = *ptr;
            // rebind mem
            mem[ptr->object_ptr] = old;
            const auto len = ptr->get_len();
            for (int i = 1; i < len + 1; ++i) {
                old[i] = ptr[i];
                if (!ptr[i].is_array()) {
                    continue;
                }
                value_ptr obj;
                try {
                    obj = mem.at(ptr[i].object_ptr);
                } catch (std::runtime_error &e) {
                    std::cerr << e.what() << std::endl;
                    exit(EXIT_FAILURE);
                }
                // use to handle cycling:
                // assert(ptr[i].object_ptr != ptr->object_ptr);
                rebind_mem(obj);
            }
            old_roots.push_back(old->object_ptr);
        }

        void minor_gc() {
            for (uint16_t i = 0; i < young_roots.size(); ++i) {
                auto *ptr = young_roots[i];
                ptr->unmark();
            }
            mark();
            // throw survivors to old arena
            vis.clear();
            for (uint16_t i = 0; i < young_roots.size(); ++i) {
                if (auto *ptr = young_roots[i]; ptr->is_marked()) {
                    assert(ptr != nullptr);
                    ptr->unmark();
                    this->rebind_mem(ptr);
                }
            }

            reset_young();

            if (old_roots.size() >= MAJOR_THRESHOLD) {
                major_gc();
            }
        }

        void large_gc() {
            unmark<interpreter::Value>(ALL(large_roots));
            mark();
            // mb rewrite later
            DynamicRoot keep_large;
            for (auto hdr: large_roots) {
                if (hdr->is_marked()) {
                    hdr->unmark();
                    keep_large.push_back(hdr);
                } else {
                    mem.erase(hdr->object_ptr);
                    large_alloc.deallocate(hdr, hdr->get_len() + 1);
                }
            }
            large_roots.swap(keep_large);
        }

        void major_gc() {
            // unmark<obj_ptr_type>(ALL(old_roots));
            for (value_ptr ptr: old_roots
                                | std::ranges::views::transform(
                    [](const uint32_t obj_ptr) -> value_ptr {
                        return mem.at(obj_ptr);
                    })
                    ) { ptr->unmark(); }

            mark();
            // mb rewrite later
            std::vector<uint32_t> survivors;
            for (auto obj_ptr: old_roots) {
                auto *ptr = mem.at(obj_ptr);
                if (ptr->is_marked()) {
                    ptr->unmark();
                    survivors.push_back(obj_ptr);
                } else {
                    mem.erase(ptr->object_ptr);
                    old_alloc.deallocate(ptr, ptr->get_len() + 1);
                }
            }
            old_roots.swap(survivors); // constant complexity :)
        }

        GarbageCollector() : stack_(nullptr), call_stack_(nullptr), fp_(nullptr) {
        }

        value_ptr alloc_array(const size_t len) {
            if (large_roots.size() >= LARGE_THRESHOLD) {
                large_gc();
            }

            value_ptr ptr;

            if (len + 1 >= YOUNG_THRESHOLD) {
                ptr = alloc_large(len);
                for (int i = 1; i < len + 1; ++i) {
                    ptr[i].set_nil();
                }
                return ptr;
            }
            if (young_alloc.get_used() + len + 1 >= YOUNG_THRESHOLD) {
                minor_gc();
            }

            ptr = alloc_young(len);
            for (int i = 1; i < len + 1; ++i) {
                ptr[i].set_nil();
            }

            return ptr;
        }

        void call(
                // interpreter::Value *stack, const uint32_t sp
        ) {
            large_gc();
            major_gc();
        }

        void cleanup() {
            reset_young();
            old_roots.clear();
            large_roots.clear();
            vis.clear();
        }
    };
} // heap

#endif //HEAP_H
