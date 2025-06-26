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

#include "value.h"

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

            [[nodiscard]] uint16_t get_used() const {
                return used;
            }
        };

        // class monotonic_resource final : public std::pmr::monotonic_buffer_resource {
        //     std::size_t used_values_ = 0;
        //
        // public:
        //     monotonic_resource(void *buf, const std::size_t buf_size)
        //         : monotonic_buffer_resource(buf, buf_size) {
        //         // reset_used();
        //     }
        //
        //
        //
        // protected:
        //     void *do_allocate(const std::size_t bytes, const std::size_t alignment) override {
        //         // this->used_values_ += bytes;
        //         return monotonic_buffer_resource::do_allocate(bytes, alignment);
        //     }
        // };

        size_t allocated_ = 1;

        // static constexpr uint16_t YOUNG_THRESHOLD = 50;
        size_t MAJOR_THRESHOLD = 10;
        size_t LARGE_THRESHOLD = 10;

        std::pmr::unsynchronized_pool_resource old_arena{};

        // Allocators
        using Alloc = std::pmr::polymorphic_allocator<interpreter::Value>;
        YoungArena young_alloc = YoungArena();
        Alloc large_alloc{std::pmr::new_delete_resource()};
        Alloc old_alloc{&old_arena};

        using value_ptr = interpreter::Value *;

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
        using DynamicRoot = std::vector<value_ptr>;
        YoungRoot young_roots = YoungRoot();
        DynamicRoot large_roots;
        DynamicRoot old_roots;

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
                return fp_? *fp_  : 0;
            }

            return (fp_? *fp_ : 0) + call_stack_->top().cur_func->max_stack;
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
            if (ptr->object_ptr == 1) {

            }
            if (ptr->object_ptr == 2) {

            }
            if (ptr->object_ptr == 3) {

            }
            if (ptr->object_ptr == 4) {

            }
            if (ptr->object_ptr == 5) {

            }
            if (ptr->object_ptr == 6) {

            }
            // auto* ptr = mem.at(object_ptr);
            // if (ptr->is_marked()) return;
            uint32_t len = ptr->get_len();
            ptr->mark();
            // for (auto *elem = ptr + 1; ptr < elem + len; ++ptr) {
            //     mark(elem);
            // }
            for (int i = 1; i < len + 1; ++i) {
                auto* neigh = ptr + i;
                if (neigh->is_array()) {
                    auto* obj_neigh = mem.at(neigh->object_ptr);
                    mark(obj_neigh);
                }
            }
        }

        // value_ptr extract_mem(uint32_t obj_ptr) {
        //     return mem.at(obj_ptr);
        // }

        void mark() const {
            assert(stack_ != nullptr || get_sp() == 0);
            try {
                for (int i = 0; i < get_sp(); ++i) {
                    if (stack_[i].is_array()) {
                        auto* ptr = mem.at(stack_[i].object_ptr);
                        ptr->unmark();
                        // ptr->mark();
                        mark(ptr);
                        // mark(stack_ + i);
                    }
                }
            } catch (std::runtime_error& e) {
                std::cerr << e.what() << std::endl;
                exit(1);
            }
        }

        void rebind_mem(value_ptr ptr, value_ptr old) {
            auto len = ptr->get_len();
            for (int i = 1; i < len + 1; ++i) {
                mem[ptr[i].object_ptr] = old + i;
                old[i] = ptr[i];
            }
        }

        void minor_gc() {
            mark();
            // throw survivors to old arena
            // for (value_ptr ptr: young_roots) {
            for (uint16_t i = 0; i < young_roots.size(); ++i) {
                if (auto *ptr = young_roots[i]; ptr->is_marked()) {
                    assert(ptr != nullptr);
                    ptr->unmark();
                    auto *old = old_alloc.allocate(ptr->get_len() + 1);
                    this->rebind_mem(ptr, old);
                    // rebind ptr
                    *old = *ptr;
                    mem[old->object_ptr] = old;
                    // *old = *ptr;
                    // old->type_part = ptr->type_part;
                    // old->set_array(ptr->get_len(), old);
                    old_roots.push_back(old);
                }
            }

            reset_young();

            if (old_roots.size() >= MAJOR_THRESHOLD) {
                major_gc();
            }
        }

        void large_gc() {
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
            // test_inner
            if (old_roots.size() == 2) {

            }

            mark(); // not needed
            // mb rewrite later
            DynamicRoot survivors;
            for (value_ptr ptr: old_roots) {
                if (ptr->is_marked()) {
                    ptr->unmark();
                    survivors.push_back(ptr);
                } else {
                    mem.erase(ptr->object_ptr);
                    old_alloc.deallocate(ptr, ptr->get_len() + 1);
                }
            }
            old_roots.swap(survivors); // constant complexity :)
        }

        GarbageCollector(): stack_(nullptr), call_stack_(nullptr), fp_(nullptr) {
            // young_roots.reserve(YOUNG_THRESHOLD >> 1);
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
            // stack_ = stack; sp_ = sp;

            // minor_gc();

            // if (large_roots.size() >= LARGE_THRESHOLD) {
            large_gc();
            // }

            // if (old_roots.size() >= MAJOR_THRESHOLD) {
            major_gc();
            // }
        }
    };
} // heap

#endif //HEAP_H
