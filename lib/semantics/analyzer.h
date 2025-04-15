//
// Created by Георгий on 14.04.2025.
//

#ifndef AST_ITERATOR_H
#define AST_ITERATOR_H

#include "exception.h"
#include "../ast.h"

using namespace ast;

namespace analysis {

class semantic_analyzer {

};

void analyze(const std::unique_ptr<ast::Node> &node);
// anyli
void analyze(const std::unique_ptr<ast::Block> &node) {
    // auto* block = node.get();
    for (const auto &child: node->lines) {
        analyze(child);
    }
}

int T = 0;

auto analyze(const std::unique_ptr<ast::Node> &node) -> void {
    if (!node) return;
    T++;

    // First visit all children (post-order traversal)
    switch (node->get_type()) {
        case ast::NodeType::Block: {
            auto *block = dynamic_cast<ast::Block *>(node.get());
            for (const auto &child: block->lines) {
                analyze(child);
            }
            break;
        }
        case ast::NodeType::FunctionDef: {
            const auto *func_def = dynamic_cast<ast::FunctionDef *>(node.get());
            analyze(func_def->block);
            break;
        }
        case ast::NodeType::FunctionCall: {
            auto *func_call = dynamic_cast<ast::FunctionCall *>(node.get());
            analyze(func_call->name_expr);
            for (const auto &arg: func_call->args) {
                analyze(arg);
            }
            break;
        }
        case ast::NodeType::ArrayGet: {
            auto *array_get = dynamic_cast<ast::ArrayGet *>(node.get());
            analyze(array_get->name_expr);
            analyze(array_get->index);
            break;
        }
        case ast::NodeType::Return: {
            auto *return_stmt = dynamic_cast<ast::ReturnStmt *>(node.get());
            analyze(return_stmt->expr);
            break;
        }
        case ast::NodeType::If: {
            auto *if_stmt = dynamic_cast<ast::IfStmt *>(node.get());
            analyze(if_stmt->expr);
            analyze(if_stmt->etrue);
            if (if_stmt->efalse) {
                analyze(if_stmt->efalse);
            }
            break;
        }
        case ast::NodeType::While: {
            throw std::runtime_error("unimplement");
        }
        case ast::NodeType::UnaryMinus: {
            auto *unary = dynamic_cast<ast::UnaryExpr<ast::UnaryOpType::MINUS> *>(node.get());
            analyze(unary->expr);
            break;
        }
        case NodeType::BinaryPlus: {
            auto *binary_add = dynamic_cast<ast::BinaryExpr<ast::BinaryOpType::ADD> *>(node.get());
            analyze(binary_add->l);
            analyze(binary_add->r);
            break;
        }
        case ast::NodeType::BinaryMul: {
            auto *binary_mul = dynamic_cast<ast::BinaryExpr<ast::BinaryOpType::MUL> *>(node.get());
            analyze(binary_mul->l);
            analyze(binary_mul->r);
            break;
        }
        case ast::NodeType::BinaryDiv: {
            const auto *binary_div = dynamic_cast<ast::BinaryExpr<ast::BinaryOpType::DIV> *>(node.get());
            analyze(binary_div->l);
            analyze(binary_div->r);
            break;
        }
        case ast::NodeType::BinaryMinus: {
            const auto *binary_sub = dynamic_cast<ast::BinaryExpr<ast::BinaryOpType::SUB> *>(node.get());
            analyze(binary_sub->l);
            analyze(binary_sub->r);
            break;
        }
        case NodeType::Assign: {
            const auto *expr = dynamic_cast<BinaryExpr<BinaryOpType::ASSIGN>*>(node.get());
            analyze(expr->l);
            analyze(expr->r);
            if (expr->l && expr->l->get_type() != NodeType::Var) {
                throw lvalue_error("lvalue can be only variable");
            }
            break;
        }
        case NodeType::IntLit:
            break;
        case NodeType::Var:
            break;
        default:
            break;
    }
}

} // analysis

#endif //AST_ITERATOR_H
