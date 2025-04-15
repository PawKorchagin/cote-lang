//
// Created by Георгий on 14.04.2025.
//

#include "analyzer.h"

using namespace ast;

namespace detail {

}

namespace analysis {
    void analyze(const std::unique_ptr<Block> &node) {
        // auto* block = node.get();
        for (const auto &child: node->lines) {
            analyze(child);
        }
    }

     void analyze(const std::unique_ptr<Node> &node) {
        if (!node) return;

        // First visit all children (post-order traversal)
        switch (node->get_type()) {
            case NodeType::Block: {
                auto *block = dynamic_cast<Block *>(node.get());
                for (const auto &child: block->lines) {
                    analyze(child);
                }
                break;
            }
            case NodeType::FunctionDef: {
                const auto *func_def = dynamic_cast<FunctionDef *>(node.get());
                analyze(func_def->block);
                break;
            }
            case NodeType::FunctionCall: {
                auto *func_call = dynamic_cast<FunctionCall *>(node.get());
                analyze(func_call->name_expr);
                for (const auto &arg: func_call->args) {
                    analyze(arg);
                }
                break;
            }
            case NodeType::ArrayGet: {
                auto *array_get = dynamic_cast<ArrayGet *>(node.get());
                analyze(array_get->name_expr);
                analyze(array_get->index);
                break;
            }
            case NodeType::Return: {
                auto *return_stmt = dynamic_cast<ReturnStmt *>(node.get());
                analyze(return_stmt->expr);
                break;
            }
            case NodeType::If: {
                auto *if_stmt = dynamic_cast<IfStmt *>(node.get());
                analyze(if_stmt->expr);
                analyze(if_stmt->etrue);
                if (if_stmt->efalse) {
                    analyze(if_stmt->efalse);
                }
                break;
            }
            case NodeType::While: {
                throw std::runtime_error("unimplement");
            }
            case NodeType::UnaryMinus: {
                auto *unary = dynamic_cast<UnaryExpr<UnaryOpType::MINUS> *>(node.get());
                analyze(unary->expr);
                break;
            }
            case NodeType::BinaryPlus: {
                auto *binary_add = dynamic_cast<BinaryExpr<BinaryOpType::ADD> *>(node.get());
                analyze(binary_add->l);
                analyze(binary_add->r);
                break;
            }
            case NodeType::BinaryMul: {
                auto *binary_mul = dynamic_cast<BinaryExpr<BinaryOpType::MUL> *>(node.get());
                analyze(binary_mul->l);
                analyze(binary_mul->r);
                break;
            }
            case NodeType::BinaryDiv: {
                const auto *binary_div = dynamic_cast<BinaryExpr<BinaryOpType::DIV> *>(node.get());
                analyze(binary_div->l);
                analyze(binary_div->r);
                break;
            }
            case NodeType::BinaryMinus: {
                const auto *binary_sub = dynamic_cast<BinaryExpr<BinaryOpType::SUB> *>(node.get());
                analyze(binary_sub->l);
                analyze(binary_sub->r);
                break;
            }
            case NodeType::Assign: {
                const auto *expr = dynamic_cast<BinaryExpr<BinaryOpType::ASSIGN> *>(node.get());
                analyze(expr->l);
                analyze(expr->r);

                // ?
                if (!expr->l) {
                    throw lvalue_error("no lvalue presented");
                }

                // ?
                if (!expr->r) {
                    throw rvalue_error("no rvalue presented");
                }

                if (expr->l && expr->l->get_type() != NodeType::Var) {
                    throw lvalue_error("lvalue can be only variable");
                }

                const auto* var = dynamic_cast<VarExpr*>(expr->l.get());

                if (var->type == VarType::UNKNOWN) {
                    throw lvalue_error("unknow type of lvalue");
                }

                break;
            }
            case NodeType::IntLit:
                break;
            case NodeType::Var:
                const auto *var = dynamic_cast<VarExpr*>(node.get());
                switch (var->type) {
                    case VarType::UNKNOWN:
                        break;
                    case VarType::INT:
                        break;
                    default:
                        throw std::runtime_error("unimplemented");
                }
                break;
        }
    }
} // analysis