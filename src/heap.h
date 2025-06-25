//
// Created by Георгий on 22.06.2025.
//

#ifndef HEAP_H
#define HEAP_H

#include <cassert>
#include <vector>
#include <memory_resource>
#include <cstddef>
#include <map>

#include "value.h"

namespace heap {
    inline std::map<uint32_t, interpreter::Value*> mem{};

    class GarbageCollector {
        class YoungArena {
            interpreter::Value* arena = nullptr;
            uint16_t used = 0;

            void alloc_buffer() {
                arena = new interpreter::Value[YOUNG_BUFFER];
            }


        public:
            YoungArena() {
                alloc_buffer();
            }

            interpreter::Value* allocate(size_t values) {
                assert(used + values < YOUNG_BUFFER);
                if (arena == nullptr) {
                    alloc_buffer();
                }
                assert(arena != nullptr && "failed to alloc arena");
                auto* res = arena + used;
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

        size_t allocated_ = 0;

        static constexpr uint16_t YOUNG_BUFFER = 50;
        static constexpr size_t MAJOR_THRESHOLD = 10;
        static constexpr size_t LARGE_THRESHOLD = 0;

        static std::byte young_buffer[YOUNG_BUFFER * sizeof(interpreter::Value)];

    protected:
        // std::pmr::monotonic_buffer_resource young_arena {
        //     young_buffer, sizeof(young_buffer)
        // };
        std::pmr::unsynchronized_pool_resource old_arena;
    private:

        // Allocators
        using Alloc = std::pmr::polymorphic_allocator<interpreter::Value>;
        YoungArena young_alloc = YoungArena();
        Alloc large_alloc{std::pmr::new_delete_resource()};
        Alloc old_alloc{&old_arena};

    protected: // test inheritance
        using value_ptr = interpreter::Value *;
        // Roots
        using Roots = std::vector<value_ptr>;
        Roots young_roots;
        Roots large_roots;
        Roots old_roots;

    private:
        interpreter::Value* stack_;
        uint32_t sp_;

        // reserve len + 1 objects to young arena
        value_ptr alloc_young(const std::size_t len) {
            value_ptr ptr = young_alloc.allocate(len + 1);
            assert(ptr);
            // add_used(len + 1);
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

        void mark() const {
            assert(stack_ != nullptr);
            for (int i = 0; i < sp_; ++i) {
                if (stack_[i].is_array()) {
                    stack_[i].mark();
                }
            }
        }

        void minor_gc() {
            mark();
            // throw survivors to old arena
            for (value_ptr ptr: young_roots) {
                if (ptr->is_marked()) {
                    ptr->unmark();
                    mem.erase(ptr->object_ptr);
                    auto* old = old_alloc.allocate(ptr->get_len() + 1);
                    old_roots.push_back(old);
                }
            }

            reset_young();
        }

        void large_gc() {
            // mb rewerite later
            Roots keep_large;
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
            // mb rewrite later
            Roots survivors;
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

    public:
        GarbageCollector(): stack_(nullptr), sp_(0) {
            young_roots.reserve(YOUNG_BUFFER >> 1);
        }

        [[nodiscard]] std::size_t get_used_values() const {
            // assert(used_values_ % sizeof(interpreter::Value) == 0);
            return young_alloc.get_used();
        }

        value_ptr alloc_array(const size_t len) {
            if (len + 1 >= YOUNG_BUFFER) {
                return alloc_large(len);
            }
            if (young_alloc.get_used() + len + 1 >= YOUNG_BUFFER) {
                minor_gc();
            }

            return alloc_young(len);
        }

        void call(interpreter::Value* stack, const uint32_t sp) {
            stack_ = stack;
            sp_ = sp;
            // mark(stack, sp); // mb
            // mark();

            minor_gc();

            if (large_roots.size() > LARGE_THRESHOLD) {
                large_gc();
            }

            if (old_roots.size() > MAJOR_THRESHOLD) {
                major_gc();
            }
        }

        // TESTING:

        size_t get_young_root_size() {
            return young_roots.size();
        }

        size_t get_old_roots_size() {
            return old_roots.size();
        }

        enum class RootsType {
            YOUNG,
            OLD,
            LARGE
        };

        value_ptr get_obj(const RootsType type, size_t idx) {
            if (type == RootsType::YOUNG)
                return mem.at(young_roots[idx]->object_ptr);

            if (type == RootsType::OLD)
                return mem.at(young_roots[idx]->object_ptr);

            return mem.at(large_roots[idx]->object_ptr);
        }

        size_t get_used_young_arena() const {
            return get_used_values();
        }
    };


    // #ifdef GC_TEST
    //
    //     inline std::vector<std::shared_ptr<interpreter::Value[]> > heap;
    // #else
    //     inline std::vector<interpreter::Value*> heap;
    // #endif
    //
    //
    //     // inline auto &get_heap() {
    //     //     return heap;
    //     // }
    //     //
    //     inline auto &get_heap(const size_t addr, const bool nullable = false) {
    //         if (addr >= heap.size()) {
    //             throw std::out_of_range("out of heap range while getting heap by addr");
    //         }
    //
    //         if (!nullable) {
    // #ifdef GC_TEST
    //             const auto *obj = heap[addr].get();
    // #else
    //             const auto* obj = heap[addr];
    // #endif
    //
    //             if (obj == nullptr) {
    //                 throw std::runtime_error("get non nullable heap addr, but got nullptr obj");
    //             }
    //         }
    //
    //         return heap[addr];
    //     }

    // size_t get_size();
    //
    // inline auto is_empty() {
    //     return heap.empty();
    // }
} // heap

#endif //HEAP_H
