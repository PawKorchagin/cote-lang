//
// Created by motya on 19.06.2025.
//
#include "lang_stdlib.h"

void cote_print(interpreter::VMData& data) {
    throw std::runtime_error("reached here: lang_stdlib.cpp");
}

void cote_stdlib::initStdlib(interpreter::VMData &data, parser::VarManager &vars) {
    data.natives[vars.add_native("print")] = cote_print;
}