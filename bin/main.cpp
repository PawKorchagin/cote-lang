#include <iostream>
#include <sstream>
#include <fstream>

#include "lib/parser.h"
#include "lib/equals.h"

#include "lib/exceptions.h"
#include "semantics/analyzer.h"


using namespace parser;
using namespace ast;

inline auto parse(const std::string &text) {
    auto in = std::stringstream(text);
    parser::init_parser(in);
    auto expr = parse_program();
    return expr;
}

void print_post_order(const std::unique_ptr<ast::Node> &node);

void print_post_order(const std::unique_ptr<ast::Block> &node) {
    // auto* block = node.get();
    for (const auto &child: node->lines) {
        print_post_order(child);
    }
}

int T = 0;

auto print_post_order(const std::unique_ptr<ast::Node> &node) -> void {
    if (!node) return;
    T++;

    // First visit all children (post-order traversal)
    switch (node->get_type()) {
        case ast::NodeType::Block: {
            auto *block = dynamic_cast<ast::Block *>(node.get());
            for (const auto &child: block->lines) {
                print_post_order(child);
            }
            break;
        }
        case ast::NodeType::FunctionDef: {
            const auto *func_def = dynamic_cast<ast::FunctionDef *>(node.get());
            print_post_order(func_def->block);
            break;
        }
        case ast::NodeType::FunctionCall: {
            auto *func_call = dynamic_cast<ast::FunctionCall *>(node.get());
            print_post_order(func_call->name_expr);
            for (const auto &arg: func_call->args) {
                print_post_order(arg);
            }
            break;
        }
        case ast::NodeType::ArrayGet: {
            auto *array_get = dynamic_cast<ast::ArrayGet *>(node.get());
            print_post_order(array_get->name_expr);
            print_post_order(array_get->index);
            break;
        }
        case ast::NodeType::Return: {
            auto *return_stmt = dynamic_cast<ast::ReturnStmt *>(node.get());
            print_post_order(return_stmt->expr);
            break;
        }
        case ast::NodeType::If: {
            auto *if_stmt = dynamic_cast<ast::IfStmt *>(node.get());
            print_post_order(if_stmt->expr);
            print_post_order(if_stmt->etrue);
            if (if_stmt->efalse) {
                print_post_order(if_stmt->efalse);
            }
            break;
        }
        case ast::NodeType::While: {
            throw std::runtime_error("unimplement");
        }
        case ast::NodeType::UnaryMinus: {
            auto *unary = dynamic_cast<ast::UnaryExpr<ast::UnaryOpType::MINUS> *>(node.get());
            print_post_order(unary->expr);
            break;
        }
        case NodeType::BinaryPlus: {
            auto *binary_add = dynamic_cast<ast::BinaryExpr<ast::BinaryOpType::ADD> *>(node.get());
            print_post_order(binary_add->l);
            print_post_order(binary_add->r);
            break;
        }
        case ast::NodeType::BinaryMul: {
            auto *binary_mul = dynamic_cast<ast::BinaryExpr<ast::BinaryOpType::MUL> *>(node.get());
            print_post_order(binary_mul->l);
            print_post_order(binary_mul->r);
            break;
        }
        case ast::NodeType::BinaryDiv: {
            auto *binary_div = dynamic_cast<ast::BinaryExpr<ast::BinaryOpType::DIV> *>(node.get());
            print_post_order(binary_div->l);
            print_post_order(binary_div->r);
            break;
        }
        case ast::NodeType::BinaryMinus: {
            auto *binary_sub = dynamic_cast<ast::BinaryExpr<ast::BinaryOpType::SUB> *>(node.get());
            print_post_order(binary_sub->l);
            print_post_order(binary_sub->r);
            break;
        }
        case NodeType::Assign: {
            auto *expr = dynamic_cast<BinaryExpr<BinaryOpType::ASSIGN>*>(node.get());
            print_post_order(expr->l);
            print_post_order(expr->r);
            break;
        }
        case NodeType::IntLit:
        case NodeType::Var:
            // leaves - no children to process
            break;
        default:
            break;
    }

    // Print node type
    switch(node->get_type()) {
        case ast::NodeType::Block: std::cout << "Block\n"; break;
        case ast::NodeType::FunctionDef: std::cout << "FunctionDef\n"; break;
        case ast::NodeType::FunctionCall: std::cout << "FunctionCall\n"; break;
        case ast::NodeType::ArrayGet: std::cout << "ArrayGet\n"; break;
        case ast::NodeType::Return: std::cout << "Return\n"; break;
        case ast::NodeType::IntLit: std::cout << "IntLit\n"; break;
        case ast::NodeType::Var: std::cout << "Var\n"; break;
        case ast::NodeType::If: std::cout << "If\n"; break;
        case ast::NodeType::While: std::cout << "While\n"; break;
        case ast::NodeType::UnaryMinus: std::cout << "UnaryMinus\n"; break;
        case ast::NodeType::BinaryPlus: std::cout << "BinaryPlus\n"; break;
        case ast::NodeType::BinaryMul: std::cout << "BinaryMul\n"; break;
        case ast::NodeType::BinaryDiv: std::cout << "BinaryDiv\n"; break;
        case ast::NodeType::BinaryMinus: std::cout << "BinaryMinus\n"; break;
        case NodeType::Assign: std::cout << "Assign\n"; break;
        default: std::cout << "Unknown\n"; break;
    }
}

// void print_post_order(const ast::Program& program) {
//     for (const auto& decl : program.declarations) {
//         print_post_order(decl);
//     }
// }

struct ex {
    int x;
};

int main() {
    using namespace ast;

    for (const auto &program = parse("fn kek() { 1 = 1; }\nfn main() { kek(); }");
         const auto &fn: program.declarations) {
        // Use reference
        std::cout << "tree start\n";
        auto tree = fn->clone_ptr_upcasting();
        // print_post_order(tree);
        analysis::analyze(tree);
    }

    // Print expression
    // std::cout << "Expression: " << mulExpr->to_str1() << std::endl;
    std::cout << T << std::endl;

    return 0;
}
