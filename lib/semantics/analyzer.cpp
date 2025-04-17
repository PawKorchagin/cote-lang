//
// Created by Георгий on 14.04.2025.
//

#include "analyzer.h"

using namespace ast;

namespace detail {

}

namespace analysis {
    // void analyze(const std::unique_ptr<Block> &node) {
    //     // auto* block = node.get();
    //     for (const auto &child: node->lines) {
    //         analyze(child);
    //     }
    // }

    std::unique_ptr<Node> analyze(std::unique_ptr<Node> node) {
        if (!node) return nullptr;

        // First visit all children (post-order traversal)
        switch (node->get_type()) {
            case NodeType::Block: {
                auto *block = dynamic_cast<Block *>(node.get());
                for (auto&& child: block->lines) {
                    analyze(std::move(child));
                }
                break;
            }
            case NodeType::FunctionDef: {
                auto *func_def = dynamic_cast<FunctionDef *>(node.get());
                analyze(std::move(func_def->block));
                break;
            }
            case NodeType::FunctionCall: {
                auto *func_call = dynamic_cast<FunctionCall *>(node.get());
                analyze(std::move(func_call->name_expr));
                for (auto&& arg: func_call->args) {
                    analyze(std::move(arg));
                }
                break;
            }
            case NodeType::ArrayGet: {
                auto *array_get = dynamic_cast<ArrayGet *>(node.get());
                analyze(std::move(array_get->name_expr));
                analyze(std::move(array_get->index));
                break;
            }
            case NodeType::Return: {
                auto *return_stmt = dynamic_cast<ReturnStmt *>(node.get());
                analyze(std::move(return_stmt->expr));
                break;
            }
            case NodeType::If: {
                auto *if_stmt = dynamic_cast<IfStmt *>(node.get());
                analyze(std::move(if_stmt->expr));
                analyze(std::move(if_stmt->etrue));
                if (if_stmt->efalse) {
                    analyze(std::move(if_stmt->efalse));
                }
                break;
            }
            case NodeType::While: {
                throw std::runtime_error("unimplement");
            }
            case NodeType::UnaryMinus: {
                auto *unary = dynamic_cast<UnaryExpr<UnaryOpType::MINUS> *>(node.get());
                analyze(std::move(unary->expr));
                break;
            }
            case NodeType::BinaryPlus: {
                auto *binary_add = dynamic_cast<BinaryExpr<BinaryOpType::ADD> *>(node.get());
                analyze(std::move(binary_add->l));
                analyze(std::move(binary_add->r));
                break;
            }
            case NodeType::BinaryMul: {
                auto *binary_mul = dynamic_cast<BinaryExpr<BinaryOpType::MUL> *>(node.get());
                analyze(std::move(binary_mul->l));
                analyze(std::move(binary_mul->r));
                break;
            }
            case NodeType::BinaryDiv: {
                auto *binary_div = dynamic_cast<BinaryExpr<BinaryOpType::DIV> *>(node.get());
                analyze(std::move(binary_div->l));
                analyze(std::move(binary_div->r));
                break;
            }
            case NodeType::BinaryMinus: {
                auto *binary_sub = dynamic_cast<BinaryExpr<BinaryOpType::SUB> *>(node.get());
                analyze(std::move(binary_sub->l));
                analyze(std::move(binary_sub->r));
                break;
            }
            case NodeType::Assign: {
                auto *expr = dynamic_cast<BinaryExpr<BinaryOpType::ASSIGN> *>(node.get());
                auto left = analyze(std::move(expr->l));
                auto right = analyze(std::move(expr->r));

                // ?
                if (!left) {
                    throw lvalue_error("no lvalue presented");
                }

                // ?
                if (!right) {
                    throw rvalue_error("no rvalue presented");
                }

                if (left && left->get_type() != NodeType::Var) {
                    throw lvalue_error("lvalue can be only variable, but got: TODO");
                }

                const auto* var = dynamic_cast<VarExpr*>(left.get());

                // var can be unknown
                // if (var->type == VarType::UNKNOWN) {
                //     throw lvalue_error("unknow type of lvalue");
                // }

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

        return node;
    }
} // analysis