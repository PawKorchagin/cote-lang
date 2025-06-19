//
// Created by motya on 07.06.2025.
//
#include "parser.h"
#include "expr_semantic.h"
#include "exceptions.h"

namespace {
    void eval_func_try() {

    }
}

bool
parser::eval_expr(ast::Node *expr, interpreter::BytecodeEmitter &emitter, parser::VarManager &vars) {
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
            if (!check_lvalue(expr, emitter, vars)) parser_throws(error_msg("lvalue failed"));
            auto name = std::move(dynamic_cast<FunctionCall *>(expr)->name_expr);
            int it = -1;
            if (name->get_type() == ast::NodeType::Var) {
                it = vars.get_native(dynamic_cast<ast::VarExpr *>(name.get())->name);
            }
            if (it == -1)
                eval_expr(name.get(), emitter, vars);
            int start = vars.last();
            int cnt = 0;
            for (auto &cur: dynamic_cast<FunctionCall *>(expr)->args) {
                cnt++;
                if (!eval_expr(cur.get(), emitter, vars)) {
                    return false;
                }
            }
            if (it == -1)
                emitter.emit_call(start, start + 1, cnt);
            else
                emitter.emit_native(it, start + 1, cnt);
            vars.drop(cnt);
            emitter.emit_move(vars.last(), vars.last() + 1);
            break;
        }
        case NodeType::ArrayGet: {
            if (!check_lvalue(expr, emitter, vars)) parser_throws(error_msg("lvalue failed"));
            auto cur = dynamic_cast<ast::ArrayGet *>(expr);
            eval_expr(cur->name_expr.get(), emitter, vars);
            eval_expr(cur->index.get(), emitter, vars);
            emitter.emit_arrayget(vars.last() - 1, vars.last() - 1, vars.last());
            vars.pop_var();
        }
            return parser_throws(error_msg("todo4")) != nullptr;
        case NodeType::Member:
            return parser_throws(error_msg("todo5")) != nullptr;
        case NodeType::Return:
            return parser_throws(error_msg("todo6")) != nullptr;
        case NodeType::StringLit:
            return parser_throws(error_msg("todo7")) != nullptr;
        case NodeType::IntLit:
            emitter.emit_loadi(vars.push_var(), dynamic_cast<IntLitExpr *>(expr)->number);
            break;
        case NodeType::Var: {
            std::string mname = dynamic_cast<VarExpr *>(expr)->name;
            const int res = vars.get_var(mname);
            if (res == -1) {
                const int f = vars.get_func(mname);
                if (f == -1) {
                    parser_throws(error_msg("identifier '" + mname + "' not found")) != nullptr;
                } else if (f == -2)
                    parser_throws(error_msg("illegal expression with native function " + mname));
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
            eval_expr(dynamic_cast<UnaryExpr<UnaryOpType::MINUS> *>(expr)->expr.get(), emitter, vars);
            emitter.emit_neg(vars.last(), vars.last());
            break;
        case NodeType::BinaryPlus:
            eval_expr(dynamic_cast<BinaryExpr<BinaryOpType::ADD> *>(expr)->l.get(), emitter, vars);
            eval_expr(dynamic_cast<BinaryExpr<BinaryOpType::ADD> *>(expr)->r.get(), emitter, vars);
            emitter.emit_add(vars.last() - 1, vars.last() - 1, vars.last());
            vars.pop_var();
            break;
        case NodeType::BinaryMul:
            eval_expr(dynamic_cast<BinaryExpr<BinaryOpType::MUL> *>(expr)->l.get(), emitter, vars);
            eval_expr(dynamic_cast<BinaryExpr<BinaryOpType::MUL> *>(expr)->r.get(), emitter, vars);
            emitter.emit_mul(vars.last() - 1, vars.last() - 1, vars.last());
            vars.pop_var();
            break;
        case NodeType::BinaryDiv:
            eval_expr(dynamic_cast<BinaryExpr<BinaryOpType::DIV> *>(expr)->l.get(), emitter, vars);
            eval_expr(dynamic_cast<BinaryExpr<BinaryOpType::DIV> *>(expr)->r.get(), emitter, vars);
            emitter.emit_div(vars.last() - 1, vars.last() - 1, vars.last());
            vars.pop_var();
            break;
        case NodeType::BinaryMinus:
            eval_expr(dynamic_cast<BinaryExpr<BinaryOpType::SUB> *>(expr)->l.get(), emitter, vars);
            eval_expr(dynamic_cast<BinaryExpr<BinaryOpType::SUB> *>(expr)->r.get(), emitter, vars);
            emitter.emit_sub(vars.last() - 1, vars.last() - 1, vars.last());
            vars.pop_var();
            break;

        case NodeType::BinaryGR:
            eval_expr(dynamic_cast<BinaryExpr<BinaryOpType::GR> *>(expr)->l.get(), emitter, vars);
            eval_expr(dynamic_cast<BinaryExpr<BinaryOpType::GR> *>(expr)->r.get(), emitter, vars);
            emitter.emit_less(vars.last() - 1, vars.last(), vars.last() - 1);
            vars.pop_var();
            break;
        case NodeType::BinaryLE:
            eval_expr(dynamic_cast<BinaryExpr<BinaryOpType::LE> *>(expr)->l.get(), emitter, vars);
            eval_expr(dynamic_cast<BinaryExpr<BinaryOpType::LE> *>(expr)->r.get(), emitter, vars);
            emitter.emit_leq(vars.last() - 1, vars.last() - 1, vars.last());
            vars.pop_var();
            break;
        case NodeType::BinaryLS:
            eval_expr(dynamic_cast<BinaryExpr<BinaryOpType::LS> *>(expr)->l.get(), emitter, vars);
            eval_expr(dynamic_cast<BinaryExpr<BinaryOpType::LS> *>(expr)->r.get(), emitter, vars);
            emitter.emit_less(vars.last() - 1, vars.last() - 1, vars.last());
            vars.pop_var();
            break;
        case NodeType::BinaryGE:
            eval_expr(dynamic_cast<BinaryExpr<BinaryOpType::GE> *>(expr)->l.get(), emitter, vars);
            eval_expr(dynamic_cast<BinaryExpr<BinaryOpType::GE> *>(expr)->r.get(), emitter, vars);
            emitter.emit_leq(vars.last() - 1, vars.last(), vars.last() - 1);
            vars.pop_var();
            break;
        case NodeType::BinaryEQ:
            eval_expr(dynamic_cast<BinaryExpr<BinaryOpType::EQ> *>(expr)->l.get(), emitter, vars);
            eval_expr(dynamic_cast<BinaryExpr<BinaryOpType::EQ> *>(expr)->r.get(), emitter, vars);
            emitter.emit_eq(vars.last() - 1, vars.last() - 1, vars.last());
            vars.pop_var();
            break;
        case NodeType::BinaryNEQ:
            eval_expr(dynamic_cast<BinaryExpr<BinaryOpType::NEQ> *>(expr)->l.get(), emitter, vars);
            eval_expr(dynamic_cast<BinaryExpr<BinaryOpType::NEQ> *>(expr)->r.get(), emitter, vars);
            emitter.emit_eq(vars.last() - 1, vars.last() - 1, vars.last());
            //TODO: NOT_EQ or NOT
            throw std::runtime_error("misha implement NOT_EQ or NOT");
//            emitter.emit_not()
            vars.pop_var();
            break;
    }
    return true;
}

bool parser::check_lvalue(ast::Node *node, interpreter::BytecodeEmitter &emitter,
                          parser::VarManager &vars) {
    const auto mtype = node->get_type();
    if (mtype == ast::NodeType::Var) return true;
    if (mtype == ast::NodeType::FunctionCall)
        return check_lvalue(dynamic_cast<ast::FunctionCall *>(node)->name_expr.get(), emitter, vars);
    if (mtype == ast::NodeType::ArrayGet)
        return check_lvalue(dynamic_cast<ast::ArrayGet *>(node)->name_expr.get(), emitter, vars);
    return false;
}
