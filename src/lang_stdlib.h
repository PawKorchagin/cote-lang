//
// Created by motya on 19.06.2025.
//

#ifndef COTE_LANG_STDLIB_H
#define COTE_LANG_STDLIB_H
#include "vm.h"
#include "var_manager.h"
namespace cote_stdlib {
    void initStdlib(interpreter::VMData& data, parser::VarManager& vars);
}

#endif //COTE_LANG_STDLIB_H
