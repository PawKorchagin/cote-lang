//
// Created by Георгий on 14.04.2025.
//

#pragma once

#ifndef AST_ITERATOR_H
#define AST_ITERATOR_H

#include <iostream>

#include "exception.h"
#include "../ast.h"


using namespace ast;

namespace detail {

}

namespace analysis {
    class semantic_analyzer {
    };

    std::unique_ptr<Node> analyze(std::unique_ptr<Node>);

    // void analyze(const std::unique_ptr<Block> &);
} // analysis

#endif //AST_ITERATOR_H
