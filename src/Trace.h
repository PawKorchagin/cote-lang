//
// Created by motya on 20.06.2025.
//

#ifndef CRYPT_TRACE_H
#define CRYPT_TRACE_H

#include "vm.h"

namespace jit {

    enum TraceResult {
        TRACE_FINISH,
        TRACE_ABORT,
    };

    class Trace {
        void record_add();
    };
}


#endif //CRYPT_TRACE_H
