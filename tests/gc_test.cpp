//
// Created by Георгий on 21.06.2025.
//

#include <utility>

#include "utils.h"

#include "gc.h"
#include "vm.h"

using namespace interpreter;

class GCTest : public Test {
protected:
    VMData vm;

    void SetUp() override {
        vm.heap_size = 0;
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
        fields[0].set_int(size);

        vm.heap[vm.heap_size] = fields;

        Value arrVal;
        arrVal.set_obj(1, vm.heap_size++);

        if (stack_idx >= 0) {
            vm.stack[stack_idx] = arrVal;
        }

        return fields;
    }
};

TEST_F(GCTest, UnreferencedArrayIsCollected) {
    std::shared_ptr ptr = allocArray(2, -1);
    const std::weak_ptr weak = ptr;
    ASSERT_EQ(vm.heap_size, 1u);
    EXPECT_EQ(vm.heap[0].get(), ptr.get());

    gc::call(vm);

    ptr.reset();

    EXPECT_EQ(vm.heap[0], nullptr);
    EXPECT_TRUE(weak.expired());
}

TEST_F(GCTest, ReferencedArrayIsKeptWithElements) {
    const std::shared_ptr ptr = allocArray(2, 0);

    ASSERT_EQ(vm.heap_size, 1u);

    gc::call(vm);

    ASSERT_TRUE(ptr.use_count() == 2);
}
