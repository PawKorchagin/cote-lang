// ////
// //// Created by motya on 06.06.2025.
// ////
// #include "utils.h"
// #include "src/ast.h"
// #include "src/JITRuntime.h"
// #include "libs/asmjit/src/asmjit/x86.h"
//
// using namespace interpreter;
// using SimpleJitTest = Test;
//
// TEST(SimpleJitTest, Test1) {
//     VMData &vm = vm_instance();
//     initVM();
//     Value v;
//     v.set_int(2);
//     vm.constanti.push_back(v);
//     v.set_int(3);
//     vm.constanti.push_back(v);
//     vm.code[vm.code_size++] = opcode(OP_LOADINT, 0, 0);
// //    vm.code[vm.code_size++] = opcode(OP_ADD, 1, 1);
// //    vm.code[vm.code_size++] = opcode(OP_ADD, 0, 0, 1);
//     vm.code[vm.code_size++] = opcode(OP_RETURN, 0, 0, 0);
//     vm.functions[0].code_size = vm.code_size;
//     jit::JITRuntime rt;
//     jit::FuncCompiled func;
//     vm.functions[0].arity = 0;
//     rt.compile(vm, vm.functions[0], func);
//     Value res;
//     uint64_t example;
//     res.set_int(5);
//
//     auto temp = *reinterpret_cast<uint64_t *>(&res);
// //    vm.stack[0].set_int(2);
// //    vm.stack[1].set_int(3);
//     ASSERT_EQ(func(vm.stack, &vm.functions[0]), temp);
// }