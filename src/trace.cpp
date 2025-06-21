//
// Created by motya on 20.06.2025.
//

#include "trace.h"


void jit::TraceEntry::run(int idx) {

}

int jit::TraceEntry::try_entry(interpreter::VMData &vm) {
    return 0;
}

jit::Trace::Trace(interpreter::VMData &vm) : vm(vm) {

}
