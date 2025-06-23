//
// Created by Георгий on 21.06.2025.
//

#include <utility>

#include "utils.h"

#include "gc.h"

#include <trace.h>

#include "heap.h"
#include "vm.h"

#define GC_TEST

using namespace interpreter;

class GCTest : public Test {
protected:
    VMData vm;

    void SetUp() override {
        // vm.heap_size = 0;
        vm.fp = 0;
    }

    /*
    void op_alloc(VMData &vm, uint8_t dst, uint8_t s) {
        if (vm.heap_size >= HEAP_MAX_SIZE) {
            throw std::runtime_error("Heap overflow");
        }

        uint32_t size = vm.stack[s].as.i32;

        auto fields = static_cast<Value *>(malloc((size + 1) * sizeof(Value)));
        if (!fields) {
            throw std::runtime_error("Memory allocation failed");
        }


        // Set len field
        fields[0].type = ValueType::Int;
        fields[0].as.i32 = size;

        vm.heap[vm.heap_size] = fields;

        Value obj_val;
        obj_val.type = ValueType::Object;
        obj_val.as.object_ptr = vm.heap_size++;
        obj_val.class_ptr = 0;  // Array class is always at index 0

        vm.stack[vm.fp + dst] = obj_val;
    }
     */

    std::shared_ptr<Value[]> allocArray(const int size, const int stack_idx) {
    const std::shared_ptr<Value[]> fields(new Value[size + 1]);
    //     fields[0].set_int(size);
    //
    //     // vm.heap[vm.heap_size] = fields;
    //     heap::heap.push_back(fields);
    //
    //     Value arrVal;
    //     arrVal.set_obj(1, heap::heap.size() - 1);
    //
    //     if (stack_idx >= 0) {
    //         vm.stack[stack_idx] = arrVal;
    //         vm.sp++;
    //     }
    //
    return fields;
    }
};

TEST_F(GCTest, UnreferencedArrayIsCollected) {
    std::shared_ptr ptr = allocArray(2, -1);
    const std::weak_ptr weak = ptr;
    ASSERT_EQ(heap::heap.size(), 1u);
    // EXPECT_EQ(heap::heap[0].get(), ptr.get());

    gc::call(vm);

    ptr.reset();

    EXPECT_EQ(heap::heap[0], nullptr);
    EXPECT_TRUE(weak.expired());
}

TEST_F(GCTest, ReferencedArrayIsKeptWithElements) {
    const std::shared_ptr ptr = allocArray(2, 0);

    ASSERT_EQ(heap::heap.size(), 1u);

    gc::call(vm);

    ASSERT_TRUE(ptr.use_count() == 2);
}

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
    gc::call(vm, true);
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
