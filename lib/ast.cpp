//
// Created by motya on 30.03.2025.
//
#include "ast.h"

namespace ast {

std::string Node::toStr1() {
    throw std::runtime_error("shouldn't be called");
}

std::string IntLitExpr::toStr1() {
    return std::to_string(number);
}

std::string VarExpr::toStr1() {
    return name;
}
}
