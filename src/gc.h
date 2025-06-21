//
// Created by Георгий on 20.06.2025.
//

#ifndef GC_H
#define GC_H
#include "vm.h"

// #define DEBUG
#ifdef DEBUG
#define log(x) std::cerr << x
#else
#define log(x) 1
#endif

namespace gc {

    void call(interpreter::VMData&);

// class gc {
//
// };

} // gc

#endif //GC_H
