//
// Created by Георгий on 21.06.2025.
//

#include <utility>

#include "utils.h"

#include "gc.h"

#include <__ranges/all.h>

#include "heap.h"
#include "vm.h"

using namespace interpreter;
using gc_test = Test;

TEST(gc_test, OldArenaNoPromoting) {
    VMData &vm = initVM();
    // vm.gc = heap::GarbageCollector();

    vm.fp = 0;
    // set_size
    vm.stack[0].set_int(5);
    // vm.stack[1]

    interpreter::op_alloc(vm, 1, 0);
    ASSERT_TRUE(vm.stack[1].is_array());
    ASSERT_EQ(vm.gc.young_alloc.get_used(), 6);
    ASSERT_EQ(vm.gc.young_roots.size(), 1);

    auto *young = vm.gc.young_roots[0];
    auto *from_stack = heap::mem.at(vm.stack[1].object_ptr);

    ASSERT_EQ(young, from_stack);
}

TEST(gc_test, LargeTest) {
    heap::mem.clear();
    auto gc = heap::GarbageCollector<5>();
    ASSERT_TRUE(gc.young_roots.size() == 0);
    ASSERT_TRUE(gc.large_roots.empty());
    ASSERT_TRUE(gc.old_roots.empty());
    auto* ptr = gc.alloc_array(4);
    ASSERT_EQ(gc.large_roots.size(), 1u);
    // Value*
}

TEST(gc_test, YoungVsLarge) {
    heap::mem.clear();
    auto gc = heap::GarbageCollector<5>();  // YOUNG_THRESHOLD = 5

    // 1) Массив len=3 → len+1=4 < 5 → young
    auto* small = gc.alloc_array(3);
    EXPECT_EQ(gc.young_roots.size(), 1u);
    EXPECT_TRUE(gc.large_roots.empty());

    // 2) Массив len=4 → len+1=5 >= 5 → large
    auto* big = gc.alloc_array(4);
    EXPECT_EQ(gc.large_roots.size(), 1u);
    EXPECT_EQ(gc.young_roots.size(), 1u);
}

TEST(gc_test, LargeGcEvictsUnmarked) {
    heap::mem.clear();
    auto gc = heap::GarbageCollector<5>();

    uint32_t fp = 0;

    gc.LARGE_THRESHOLD = 2;

    auto* a = gc.alloc_array(4);
    auto* b = gc.alloc_array(4);
    ASSERT_TRUE(a->is_marked());
    ASSERT_TRUE(b->is_marked());
    // здесь запускается large_gc и unmark a, b
    auto* c = gc.alloc_array(4);
    ASSERT_TRUE(!a->is_marked());
    ASSERT_TRUE(!b->is_marked());
    ASSERT_TRUE(c->is_marked());
    ASSERT_EQ(gc.large_roots.size(), 3u);
    gc.call();
    ASSERT_TRUE(!c->is_marked());
    EXPECT_EQ(gc.large_roots.size(), 1u);
    gc.call();
    EXPECT_TRUE(gc.large_roots.empty());
}

TEST(gc_test, MemMapConsistency) {
    heap::mem.clear();
    auto gc = heap::GarbageCollector<5>();

    // add young
    auto* y = gc.alloc_array(1);

    ASSERT_EQ(y->type_part, 7u);
    // add large
    auto* L = gc.alloc_array(4);
    ASSERT_EQ(L->type_part, 19u);
    ASSERT_TRUE(heap::mem.count(y->object_ptr));
    ASSERT_TRUE(heap::mem.count(L->object_ptr));
    ASSERT_EQ(L->object_ptr, 2);

    // unmark
    gc.call();
    ASSERT_TRUE(y);
    ASSERT_TRUE(L);
    ASSERT_EQ(y->type_part, 7u);
    ASSERT_EQ(L->type_part, 17u); // unmarked
    // delete
    gc.call();

    EXPECT_TRUE(heap::mem.count(y->object_ptr));

    EXPECT_FALSE(heap::mem.count(/*deleted L object ptr =*/2));

    ASSERT_EQ(gc.young_roots.size(), 1u);

    uint32_t y_object_ptr = y->object_ptr;
    ASSERT_EQ(y_object_ptr, 1u);
    gc.minor_gc(); // unmark
    // y invalids after young->old
    EXPECT_NO_THROW({ y = heap::mem.at(y_object_ptr); }); // reset y here

    // ASSERT_TRUE(y != nullptr);
    ASSERT_EQ(y->type_part, 5u); // unmarked array of len 1

    ASSERT_EQ(gc.young_roots.size(), 0u);
    ASSERT_EQ(gc.old_roots.size(), 1u);

    EXPECT_TRUE(heap::mem.count(y->object_ptr));
}

TEST(gc_test, DoubleMinorGc) {
    heap::mem.clear();
    auto gc = heap::GarbageCollector<10>();
    auto* a = gc.alloc_array(2);
    gc.minor_gc();  // промоутит всё живое в old
    gc.minor_gc();  // здесь should be no-op: young пуст, old не трогаем
    EXPECT_EQ(gc.old_roots.size(), 1u);
}

TEST(gc_test, Thresholds) {
    heap::mem.clear();
    auto gc = heap::GarbageCollector<10>();
    auto* a = gc.alloc_array(1);
    auto* b = gc.alloc_array(1);
    auto* c = gc.alloc_array(1);
    auto* d = gc.alloc_array(1);
    ASSERT_EQ(a->object_ptr, 7u);
    ASSERT_EQ(b->object_ptr, 7u);
    ASSERT_EQ(c->object_ptr, 7u);
    ASSERT_EQ(d->object_ptr, 7u);
    // no calling minor_gc before this line
    auto* e = gc.alloc_array(1); // here calling minor_gc

    gc.LARGE_THRESHOLD = 1;
    gc.MAJOR_THRESHOLD = 1;
}

// TEST(gc_test, QTest) {
//
// }

// TEST_F(GCTest, ReferencedArrayIsKeptWithElements) {
//     const std::shared_ptr ptr = allocArray(2, 0);
//
//     ASSERT_EQ(heap::heap.size(), 1u);
//
//     // gc::gc::call(vm);
//
//     ASSERT_TRUE(ptr.use_count() == 2);
// }

inline void compile_program(std::istream &fin, const std::string &file_name = "code") {
    std::cerr << "kek\n";
    using namespace interpreter;
    auto &vm = initVM();
    parser::init_parser(fin, new BytecodeEmitter());
    ast::Program p;
    ASSERT_NO_THROW(p = parser::parse_program(vm));
    print_vm_data(vm);
    //    print_func_body(p.instructions);
    interpreter::run(true);
    // gc::gc::call(vm, true);
    ASSERT_TRUE(vm.call_stack.empty());
    ASSERT_EQ(vm.stack[0].i32, 0);


    if (!parser::get_errors().empty()) {
        for (auto x: get_errors()) {
            std::cerr << file_name << ":" << x <<
                    std::endl;
        }
        throw std::runtime_error("parser failed: " + get_errors().front());
    }
}

using run_test = Test;

TEST(run_test, ArrayLink) {
    ASSERT_NO_THROW({
        std::ifstream fin("../../tests/sources/gc/test_arraylink.ct" );
        compile_program(fin);
        // ASSERT_EQ(vm.heap_size, 30u);
        });
}

TEST(run_test, ArrayLinkFromFooMini) {
    ASSERT_NO_THROW({
        std::ifstream fin("../../tests/sources/gc/test_arraylink_from_foo_mini.ct" );
        compile_program(fin);
        // ASSERT_EQ(gc::get_calls(), 2);
        // ASSERT_EQ(vm.heap_size, 30u);
        });
}
