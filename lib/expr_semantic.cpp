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
            throw std::runtime_error("not supported3 for now");
            auto name = std::move(dynamic_cast<FunctionCall *>(expr.get())->name_expr);
            if (name->get_type() != ast::NodeType::Var) throw std::runtime_error("simple function support");
            auto sname = dynamic_cast<VarExpr *>(expr.get())->name;
            if (sname != "print") throw std::runtime_error("unknown function name");
            int start = vars.last() + 1;
            int cnt = 0;
            for (auto &cur: dynamic_cast<FunctionCall *>(expr.get())->args) {
                cnt++;
                if (!eval_expr(std::move(cur), emitter, vars)) {
                    return false;
                }
            }

            emitter.emit_call(0, start, 1);

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
            const int res = vars.get_var(dynamic_cast<VarExpr *>(expr.get())->name);
            if (res == -1) return parser_throws(error_msg("variables not found")) != nullptr;
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
    switch (expr->get_type()) {
        case NodeType::Block:
            return parser_throws(error_msg("block is not allowed in expr"));
        case NodeType::FunctionDef:
            return parser_throws(error_msg("todo1"));
        case NodeType::FunctionSingature:
            return parser_throws(error_msg("todo2"));
        case NodeType::FunctionCall:
            return std::move(expr);
        case NodeType::ArrayGet:
            return parser_throws(error_msg("todo4"));
        case NodeType::Member:
            return parser_throws(error_msg("todo5"));
        case NodeType::Return:
            return parser_throws(error_msg("todo6"));
        case NodeType::StringLit:
            return parser_throws(error_msg("todo7"));
        case NodeType::IntLit:
            return std::move(expr);
        case NodeType::Var: {
            return std::move(expr);
        }
        case NodeType::If:
            return parser_throws(error_msg("todo9"));
        case NodeType::While:
            return parser_throws(error_msg("todo10"));
        case NodeType::For:
            return parser_throws(error_msg("todo11"));
        case NodeType::UnaryMinus:
        case NodeType::BinaryPlus:
        case NodeType::BinaryMul:
        case NodeType::BinaryDiv:
        case NodeType::BinaryMinus:
        case NodeType::BinaryGR:
        case NodeType::BinaryLE:
        case NodeType::BinaryLS:
        case NodeType::BinaryGE:
        case NodeType::BinaryEQ:
        case NodeType::BinaryNEQ:
            return std::move(expr);
        default:
            throw std::runtime_error("unknown error");
        case NodeType::Assign:
            throw std::runtime_error("internal error: illegal expression");
    }
}
