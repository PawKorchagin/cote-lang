//
// Created by motya on 07.06.2025.
//

#ifndef CRYPT_EXPR_SEMANTIC_H
#define CRYPT_EXPR_SEMANTIC_H

#include "bytecode_emitter.h"
#include "var_manager.h"
#include "ast.h"

namespace parser {
    bool eval_expr(std::unique_ptr<ast::Node> expr, interpreter::BytecodeEmitter &emitter, parser::VarManager &vars);

    std::unique_ptr<ast::Node>
    check_expr(std::unique_ptr<ast::Node> expr, interpreter::BytecodeEmitter &emitter, parser::VarManager &vars);

    bool check_lvalue(ast::Node *expr, interpreter::BytecodeEmitter &emitter, parser::VarManager &vars);
}

#endif //CRYPT_EXPR_SEMANTIC_H
