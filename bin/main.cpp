//#include <cassert>
//#include <iostream>
//#include <sstream>
//#include <fstream>
//#include <sys/stat.h>
//
//#include "lib/parser.h"
//#include "lib/equals.h"
//
//#include "lib/exceptions.h"
//#include "semantics/analyzer.h"

//
//
//using namespace parser;
//using namespace ast;
//
//inline auto parse(const std::string &text) {
//    auto in = std::stringstream(text);
//    parser::init_parser(in, new BytecodeEmitter());
//    auto expr = parse_program();
//    return expr;
//}
//
//void print_post_order(const std::unique_ptr<Node> &node);
//
//void print_post_order(const std::unique_ptr<Block> &node) {
//    // auto* block = node.get();
//    for (const auto &child: node->lines) {
//        print_post_order(child);
//    }
//}
//
//int T = 0;
//
//void print_post_order(const std::unique_ptr<Node> &node) {
//    if (!node) return;
//    T++;
//
//    // First visit all children (post-order traversal)
//    switch (node->get_type()) {
//        case NodeType::Block: {
//            auto *block = dynamic_cast<Block *>(node.get());
//            for (const auto &child: block->lines) {
//                print_post_order(child);
//            }
//            break;
//        }
//        case NodeType::FunctionDef: {
//            const auto *func_def = dynamic_cast<FunctionDef *>(node.get());
//            print_post_order(func_def->block);
//            break;
//        }
//        case NodeType::FunctionCall: {
//            auto *func_call = dynamic_cast<FunctionCall *>(node.get());
//            print_post_order(func_call->name_expr);
//            for (const auto &arg: func_call->args) {
//                print_post_order(arg);
//            }
//            break;
//        }
//        case NodeType::ArrayGet: {
//            auto *array_get = dynamic_cast<ArrayGet *>(node.get());
//            print_post_order(array_get->name_expr);
//            print_post_order(array_get->index);
//            break;
//        }
//        case NodeType::Return: {
//            auto *return_stmt = dynamic_cast<ReturnStmt *>(node.get());
//            print_post_order(return_stmt->expr);
//            break;
//        }
//        case NodeType::If: {
//            auto *if_stmt = dynamic_cast<IfStmt *>(node.get());
//            print_post_order(if_stmt->expr);
//            print_post_order(if_stmt->etrue);
//            if (if_stmt->efalse) {
//                print_post_order(if_stmt->efalse);
//            }
//            break;
//        }
//        case NodeType::While: {
//            throw std::runtime_error("unimplement");
//        }
//        case NodeType::UnaryMinus: {
//            auto *unary = dynamic_cast<UnaryExpr<UnaryOpType::MINUS> *>(node.get());
//            print_post_order(unary->expr);
//            break;
//        }
//        case NodeType::BinaryPlus: {
//            auto *binary_add = dynamic_cast<BinaryExpr<BinaryOpType::ADD> *>(node.get());
//            print_post_order(binary_add->l);
//            print_post_order(binary_add->r);
//            break;
//        }
//        case NodeType::BinaryMul: {
//            auto *binary_mul = dynamic_cast<BinaryExpr<BinaryOpType::MUL> *>(node.get());
//            print_post_order(binary_mul->l);
//            print_post_order(binary_mul->r);
//            break;
//        }
//        case NodeType::BinaryDiv: {
//            auto *binary_div = dynamic_cast<BinaryExpr<BinaryOpType::DIV> *>(node.get());
//            print_post_order(binary_div->l);
//            print_post_order(binary_div->r);
//            break;
//        }
//        case NodeType::BinaryMinus: {
//            auto *binary_sub = dynamic_cast<BinaryExpr<BinaryOpType::SUB> *>(node.get());
//            print_post_order(binary_sub->l);
//            print_post_order(binary_sub->r);
//            break;
//        }
//        case NodeType::Assign: {
//            auto *expr = dynamic_cast<BinaryExpr<BinaryOpType::ASSIGN> *>(node.get());
//            print_post_order(expr->l);
//            print_post_order(expr->r);
//            break;
//        }
//        case NodeType::IntLit: break;
//        case NodeType::Var: {
//            auto *var = dynamic_cast<VarExpr *>(node.get());
//            std::cout << "Var: name " << var->name << " type: " << detail::get_type(var->type) << std::endl;
//        }
//        // leaves - no children to process
//        break;
//        default:
//            break;
//    }
//
//    // Print node type
//    switch (node->get_type()) {
//        case NodeType::Block: std::cout << "Block\n";
//            break;
//        case NodeType::FunctionDef: std::cout << "FunctionDef\n";
//            break;
//        case NodeType::FunctionCall: std::cout << "FunctionCall\n";
//            break;
//        case NodeType::ArrayGet: std::cout << "ArrayGet\n";
//            break;
//        case NodeType::Return: std::cout << "Return\n";
//            break;
//        case NodeType::IntLit: std::cout << "IntLit\n";
//            break;
//        case NodeType::Var: {
//            break;
//        }
//        case NodeType::If: std::cout << "If\n";
//            break;
//        case NodeType::While: std::cout << "While\n";
//            break;
//        case NodeType::UnaryMinus: std::cout << "UnaryMinus\n";
//            break;
//        case NodeType::BinaryPlus: std::cout << "BinaryPlus\n";
//            break;
//        case NodeType::BinaryMul: std::cout << "BinaryMul\n";
//            break;
//        case NodeType::BinaryDiv: std::cout << "BinaryDiv\n";
//            break;
//        case NodeType::BinaryMinus: std::cout << "BinaryMinus\n";
//            break;
//        case NodeType::Assign: std::cout << "Assign\n";
//            break;
//        default: std::cout << "Unknown\n";
//            break;
//    }
//}
//
//// void print_post_order(const Program& program) {
////     for (const auto& decl : program.declarations) {
////         print_post_order(decl);
////     }
//// }
//
//struct ex {
//    int x;
//};
//
//int main() {
//    using namespace ast;
//
//    try {
//        for (const auto &program = parse("fn main() { 1 = 1; }");
//
//             const std::unique_ptr<FunctionDef> &fn: program.declarations) {
//            // Use reference
//            std::cout << "tree start\n";
//            auto tree = fn->move_upcasting();
//            print_post_order(tree);
//            analysis::analyze(std::move(tree));
//            // analysis::analyze(static_cast<std::unique_ptr<Node>>(fn));
//        }
//    } catch (const std::exception &e) {
//        std::cerr << e.what() << std::endl;
//    }
//    // Print expression
//    // std::cout << "Expression: " << mulExpr->to_str1() << std::endl;
//    std::cout << T << std::endl;
//
//    return 0;
//}

#include "src/parser.h"

#include "src/value.h"

int main() {
    std::cout << sizeof(interpreter::Value);
    return 0;
}
