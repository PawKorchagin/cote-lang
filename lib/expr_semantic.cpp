//
// Created by motya on 07.06.2025.
//
#include "parser.h"
#include "expr_semantic.h"
#include "exceptions.h"

namespace {

}

bool
parser::eval_expr(std::unique_ptr<ast::Node> expr, interpreter::BytecodeEmitter &emitter, parser::VarManager &vars) {
    using namespace ast;
    if (expr == nullptr) return false;
    switch (expr->get_type()) {
        case NodeType::Block:
            return parser_throws(error_msg("block is not allowed in expr")) != nullptr;
        case NodeType::FunctionDef:
            return parser_throws(error_msg("todo1")) != nullptr;
        case NodeType::FunctionSingature:
            return parser_throws(error_msg("todo2")) != nullptr;
        case NodeType::FunctionCall: {
            auto name = std::move(dynamic_cast<FunctionCall *>(expr.get())->name_expr);
            eval_expr(std::move(name), emitter, vars);
            int start = vars.last();
            int cnt = 0;
            for (auto &cur: dynamic_cast<FunctionCall *>(expr.get())->args) {
                cnt++;
                if (!eval_expr(std::move(cur), emitter, vars)) {
                    return false;
                }
            }
            emitter.emit_call(start, start + 1, cnt);
            vars.drop(cnt);
            emitter.emit_move(vars.last(), vars.last() + 1);
            break;
        }
        case NodeType::ArrayGet:
            return parser_throws(error_msg("todo4")) != nullptr;
        case NodeType::Member:
            return parser_throws(error_msg("todo5")) != nullptr;
        case NodeType::Return:
            return parser_throws(error_msg("todo6")) != nullptr;
        case NodeType::StringLit:
            return parser_throws(error_msg("todo7")) != nullptr;
        case NodeType::IntLit:
            emitter.emit_loadi(vars.push_var(), dynamic_cast<IntLitExpr *>(expr.get())->number);
            break;
        case NodeType::Var: {
            std::string mname = dynamic_cast<VarExpr *>(expr.get())->name;
            const int res = vars.get_var(mname);
            if (res == -1) {
                const int f = vars.get_func(mname);
                if (f == -1) {
                    parser_throws(error_msg("unknown identifier '" + mname + "' not found")) != nullptr;
                }
                emitter.emit_loadfunc(vars.push_var(), f);
                return true;
            }
            emitter.emit_move(vars.push_var(), res);
            break;
        }
        case NodeType::If:
            return parser_throws(error_msg("todo9")) != nullptr;
        case NodeType::While:
            return parser_throws(error_msg("todo10")) != nullptr;
        case NodeType::For:
            return parser_throws(error_msg("todo11")) != nullptr;
        case NodeType::UnaryMinus:
            eval_expr(std::move(dynamic_cast<UnaryExpr<UnaryOpType::MINUS> *>(expr.get())->expr), emitter, vars);
            emitter.emit_neg(vars.last(), vars.last());
            break;
        case NodeType::BinaryPlus:
            eval_expr(std::move(dynamic_cast<BinaryExpr<BinaryOpType::ADD> *>(expr.get())->l), emitter, vars);
            eval_expr(std::move(dynamic_cast<BinaryExpr<BinaryOpType::ADD> *>(expr.get())->r), emitter, vars);
            emitter.emit_add(vars.last() - 1, vars.last() - 1, vars.last());
            vars.pop_var();
            break;
        case NodeType::BinaryMul:
            eval_expr(std::move(dynamic_cast<BinaryExpr<BinaryOpType::MUL> *>(expr.get())->l), emitter, vars);
            eval_expr(std::move(dynamic_cast<BinaryExpr<BinaryOpType::MUL> *>(expr.get())->r), emitter, vars);
            emitter.emit_mul(vars.last() - 1, vars.last() - 1, vars.last());
            vars.pop_var();
            break;
        case NodeType::BinaryDiv:
            eval_expr(std::move(dynamic_cast<BinaryExpr<BinaryOpType::DIV> *>(expr.get())->l), emitter, vars);
            eval_expr(std::move(dynamic_cast<BinaryExpr<BinaryOpType::DIV> *>(expr.get())->r), emitter, vars);
            emitter.emit_div(vars.last() - 1, vars.last() - 1, vars.last());
            vars.pop_var();
            break;
        case NodeType::BinaryMinus:
            eval_expr(std::move(dynamic_cast<BinaryExpr<BinaryOpType::SUB> *>(expr.get())->l), emitter, vars);
            eval_expr(std::move(dynamic_cast<BinaryExpr<BinaryOpType::SUB> *>(expr.get())->r), emitter, vars);
            emitter.emit_sub(vars.last() - 1, vars.last() - 1, vars.last());
            vars.pop_var();
            break;

        case NodeType::BinaryGR:
            eval_expr(std::move(dynamic_cast<BinaryExpr<BinaryOpType::GR> *>(expr.get())->l), emitter, vars);
            eval_expr(std::move(dynamic_cast<BinaryExpr<BinaryOpType::GR> *>(expr.get())->r), emitter, vars);
            emitter.emit_less(vars.last() - 1, vars.last(), vars.last() - 1);
            vars.pop_var();
            break;
        case NodeType::BinaryLE:
            eval_expr(std::move(dynamic_cast<BinaryExpr<BinaryOpType::LE> *>(expr.get())->l), emitter, vars);
            eval_expr(std::move(dynamic_cast<BinaryExpr<BinaryOpType::LE> *>(expr.get())->r), emitter, vars);
            emitter.emit_leq(vars.last() - 1, vars.last() - 1, vars.last());
            vars.pop_var();
            break;
        case NodeType::BinaryLS:
            eval_expr(std::move(dynamic_cast<BinaryExpr<BinaryOpType::LS> *>(expr.get())->l), emitter, vars);
            eval_expr(std::move(dynamic_cast<BinaryExpr<BinaryOpType::LS> *>(expr.get())->r), emitter, vars);
            emitter.emit_less(vars.last() - 1, vars.last() - 1, vars.last());
            vars.pop_var();
            break;
        case NodeType::BinaryGE:
            eval_expr(std::move(dynamic_cast<BinaryExpr<BinaryOpType::GE> *>(expr.get())->l), emitter, vars);
            eval_expr(std::move(dynamic_cast<BinaryExpr<BinaryOpType::GE> *>(expr.get())->r), emitter, vars);
            emitter.emit_leq(vars.last() - 1, vars.last(), vars.last() - 1);
            vars.pop_var();
            break;
        case NodeType::BinaryEQ:
            eval_expr(std::move(dynamic_cast<BinaryExpr<BinaryOpType::EQ> *>(expr.get())->l), emitter, vars);
            eval_expr(std::move(dynamic_cast<BinaryExpr<BinaryOpType::EQ> *>(expr.get())->r), emitter, vars);
            emitter.emit_eq(vars.last() - 1, vars.last() - 1, vars.last());
            vars.pop_var();
            break;
        case NodeType::BinaryNEQ:
            eval_expr(std::move(dynamic_cast<BinaryExpr<BinaryOpType::NEQ> *>(expr.get())->l), emitter, vars);
            eval_expr(std::move(dynamic_cast<BinaryExpr<BinaryOpType::NEQ> *>(expr.get())->r), emitter, vars);
            emitter.emit_eq(vars.last() - 1, vars.last() - 1, vars.last());
            //TODO: NOT_EQ or NOT
            throw std::runtime_error("misha implement NOT_EQ or NOT");
//            emitter.emit_not()
            vars.pop_var();
            break;
    }
    return true;
}

std::unique_ptr<ast::Node>
parser::check_expr(std::unique_ptr<ast::Node> expr, interpreter::BytecodeEmitter &emitter, parser::VarManager &vars) {
    using namespace ast;
    if (expr == nullptr) return expr;
    auto it = expr->get_type();
    if (it == NodeType::ArrayGet || it == NodeType::FunctionCall) {
        if (!check_lvalue(expr.get(), emitter, vars))
            parser_throws("illegal expression: expected lvalue expression");
    }
    return std::move(expr);
}

bool parser::check_lvalue(ast::Node* node, interpreter::BytecodeEmitter &emitter,
                     parser::VarManager &vars) {
    const auto mtype = node->get_type();
    if (mtype == ast::NodeType::Var) return true;
    if (mtype == ast::NodeType::FunctionCall) return check_lvalue(dynamic_cast<ast::FunctionCall*>(node)->name_expr.get(), emitter, vars);
    if (mtype == ast::NodeType::ArrayGet) return check_lvalue(dynamic_cast<ast::ArrayGet*>(node)->name_expr.get(), emitter, vars);
    return false;
}
