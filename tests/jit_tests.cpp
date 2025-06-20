//
// Created by motya on 06.06.2025.
//
#include "utils.h"
#include "src/ast.h"
#include "src/nodes.h"
#include "libs/asmjit/src/asmjit/core.h"
#include "libs/asmjit/src/asmjit/x86.h"

using namespace interpreter;
using SimpleJitTest = Test;

TEST(SimpleJitTest, Test1) {
    asmjit::JitRuntime jit;
    VMData& vm = initVM();
    BytecodeEmitter* emitter = new BytecodeEmitter();
    emitter->emit_loadi(0, 1);
    emitter->emit_loadi(1, 2);
    emitter->emit_add(0, 0, 1);
    emitter->emit_return(0);
    emitter->initVM(vm);
    cfg::VMInfo vmInfo(vm);
    vmInfo.code = vm.code;
    vmInfo.code_size = vm.code_size;
    cfg::CFGraph graph(vmInfo);
    ASSERT_TRUE(graph.buildBasicCFG());
    graph.toString(std::cout);
}