//
// Created by motya on 07.06.2025.
//

#ifndef COTE_EXPR_SEMANTIC_H
#define COTE_EXPR_SEMANTIC_H

#include "bytecode_emitter.h"
#include "var_manager.h"
#include "ast.h"

namespace parser {
    bool eval_expr(ast::Node* expr, interpreter::BytecodeEmitter &emitter, parser::VarManager &vars);

    bool check_lvalue(ast::Node *expr, interpreter::BytecodeEmitter &emitter, parser::VarManager &vars);
}

#endif //COTE_EXPR_SEMANTIC_H
