//
// Created by motya on 21.06.2025.
//

#ifndef CRYPT_JITRUNTIME_H
#define CRYPT_JITRUNTIME_H

#include "asmjit/asmjit.h"
#include "trace.h"
class JITRuntime {
public:
    asmjit::JitRuntime lib_runtime;
    JITRuntime() {  }
    void parseIR() {

    }
};


#endif //CRYPT_JITRUNTIME_H
