//
// Created by Георгий on 24.06.2025.
//

#ifndef VALUE_H
#define VALUE_H

#include <cstdint>

namespace interpreter {
    static constexpr uint32_t TYPE_OBJ = 1;
    static constexpr uint32_t MARK_BIT = 2;
    static constexpr uint32_t TYPE_INT = 4;
    static constexpr uint32_t TYPE_FLOAT = 8;
    static constexpr uint32_t TYPE_CALLABLE = 12;
    static constexpr uint32_t TYPE_NIL = 16;
    static constexpr uint32_t UNMARK_BITS = ~MARK_BIT;
    static constexpr uint64_t OBJ_NIL = (uint64_t) TYPE_NIL << 32ull;

    // type_part:
<<<<<<< main
    // static constexpr uint32_t type_part_obj_ = 0b000000000000000000000000000000'0/1'1;
    //                                          | space for obj idx               |    | obj_mark
    //                                                                           gc mark
=======
    // static constexpr uint32_t type_part_obj_ = 0b00000000000000000000000000000'0'0/1'1;
    //                                          | space for obj idx                |   | obj
    //                                                                             mark
>>>>>>> dev
    // objects: xxxx y 1 - last bits means that it's an object
    //               y - mark bit
    // nil has class_type = 0
    // array has class_type = 1;
    // non-objects: xxx100 - int
    //              xx1000 - float
    //              xx1100 - callable
    //              x10000 - nil


    struct Value {
        union {
            int32_t i32;
            float f32;
            uint32_t object_ptr;
        }; // low bits
        uint32_t type_part; // high bits


        // Value()=default;
        Value() {
        }

        void mark() { type_part |= MARK_BIT; }

        void unmark() { type_part &= UNMARK_BITS; }

        // TODO simplify
        void flip_mark() {
           type_part ^= MARK_BIT;
        }

        bool is_marked() const { return type_part & MARK_BIT; }

        uint64_t as_unmarked() const {
            return (static_cast<uint64_t>(UNMARK_BITS & type_part) << 32ull) | static_cast<uint64_t>(i32);
        }

        inline int32_t get_class() const { return (type_part >> 2) & 1; } //1 for obj type; 1 for mark bit, & 1 bc size

        uint32_t get_len() {
            return type_part >> 3;
        }

        inline void set_nil() {
            type_part = TYPE_NIL;
            i32 = 0;
        }

        inline void set_int(int val) {
            type_part = TYPE_INT;
            i32 = val;
        }

        inline void set_callable(int val) {
            type_part = TYPE_CALLABLE;
            i32 = val;
        }

        inline void set_float(float val) {
            type_part = TYPE_FLOAT;
            f32 = val;
        }

        template<bool marked = false>
        inline void set_obj(const uint32_t class_info, Value *ptr_val) {
            type_part = class_info << 2ull | TYPE_OBJ | static_cast<uint32_t>(marked) << 1;
            assert(class_info == 0 || ptr_val);
            object_ptr = !class_info ? 0 : ptr_val->object_ptr; // set nullptr to nil or objectptr
        }

        template<bool marked = true>
        inline void set_array(const uint32_t size, Value *ptr_val) {
<<<<<<< main
            set_obj(size << 2ull, ptr_val);
=======
            set_obj<marked>(1, ptr_val);
            type_part |= size << 3ull;
>>>>>>> dev
        }

        inline bool is_nil() const { return as_unmarked() == OBJ_NIL; }

        inline bool is_int() const { return (type_part & UNMARK_BITS) == TYPE_INT; }

        inline bool is_float() const { return (type_part & UNMARK_BITS) == TYPE_FLOAT; }

        //TODO: add char support

        inline bool is_object() const { return type_part & 1; }

        inline bool is_array() const { return is_object(); }

        inline bool is_callable() const { return (type_part & UNMARK_BITS) == TYPE_CALLABLE; }

        //only for numeric types
        inline float cast_to_float() const {
            return is_float() ? f32 : static_cast<float>(i32);
        }

        inline uint64_t as_uint64() { return *reinterpret_cast<uint64_t *>(this); }
    };
}


#endif //VALUE_H
