//
// Created by motya on 07.06.2025.
//
#include "parser.h"
#include "expr_semantic.h"
#include "exceptions.h"

namespace {
    template<ast::BinaryOpType mtype>
    void get_func(interpreter::BytecodeEmitter &emitter, parser::VarManager &vars) {
        if constexpr (mtype == ast::BinaryOpType::ADD) {
            emitter.emit_add(vars.last() - 1, vars.last() - 1, vars.last());
        } else if constexpr (mtype == ast::BinaryOpType::DIV) {
            emitter.emit_div(vars.last() - 1, vars.last() - 1, vars.last());
        } else if constexpr (mtype == ast::BinaryOpType::MUL) {
            emitter.emit_mul(vars.last() - 1, vars.last() - 1, vars.last());
        } else if constexpr (mtype == ast::BinaryOpType::SUB) {
            emitter.emit_sub(vars.last() - 1, vars.last() - 1, vars.last());
        } else if constexpr (mtype == ast::BinaryOpType::MOD) {
            emitter.emit_mod(vars.last() - 1, vars.last() - 1, vars.last());
        } else if constexpr (mtype == ast::BinaryOpType::EQ) {
            emitter.emit_eq(vars.last() - 1, vars.last() - 1, vars.last());
        } else if constexpr (mtype == ast::BinaryOpType::NEQ) {
            emitter.emit_neq(vars.last() - 1, vars.last() - 1, vars.last());
        } else if constexpr (mtype == ast::BinaryOpType::LE) {
            emitter.emit_leq(vars.last() - 1, vars.last() - 1, vars.last());
        } else if constexpr (mtype == ast::BinaryOpType::LS) {
            emitter.emit_less(vars.last() - 1, vars.last() - 1, vars.last());
        } else if constexpr (mtype == ast::BinaryOpType::GR) {
            emitter.emit_less(vars.last() - 1, vars.last(), vars.last() - 1);
        } else if constexpr (mtype == ast::BinaryOpType::GE) {
            emitter.emit_leq(vars.last() - 1, vars.last(), vars.last() - 1);
        } else throw std::runtime_error("todo");
    }

    template<typename T>
    void simple_eval_binary(ast::Node *expr, interpreter::BytecodeEmitter &emitter, parser::VarManager &vars) {
        eval_expr(dynamic_cast<T *>(expr)->l.get(), emitter, vars);
        eval_expr(dynamic_cast<T *>(expr)->r.get(), emitter, vars);
        get_func<T::ownType()>(emitter, vars);//vars.last() - 1, vars.last() - 1, vars.last());
        vars.pop_var();
    }
}

bool
parser::eval_expr(ast::Node *expr, interpreter::BytecodeEmitter &emitter, parser::VarManager &vars) {
    using namespace ast;
    if (expr == nullptr) return false;
    switch (expr->get_type()) {
        case NodeType::FunctionCall: {
            if (!check_lvalue(expr, emitter, vars)) parser_throws(error_msg("lvalue failed"));
            auto name = dynamic_cast<FunctionCall *>(expr)->name_expr.get();
            int it = -1;
            if (name->get_type() == ast::NodeType::Var) {
                std::string &sname = dynamic_cast<ast::VarExpr *>(name)->name;
                if (sname == "array")
                    it = -2;
                else
                    it = vars.get_native(sname);
            }
            if (it == -1)
                eval_expr(name, emitter, vars);
            else
                vars.push_var();
            int start = vars.last();
            int cnt = 0;
            for (auto &cur: dynamic_cast<FunctionCall *>(expr)->args) {
                cnt++;
                if (!eval_expr(cur.get(), emitter, vars)) {
                    return false;
                }
            }
            if (it == -2) {
                if (cnt != 1)
                    parser_throws("only one argument expected in function 'array'");
                vars.drop(1);
                emitter.emit_alloc(vars.last(), start + 1);
                return true;
            }
            if (it == -1) {
                emitter.emit_call(start, start + 1, cnt);
            } else {
                emitter.emit_native(it, start + 1, cnt);
            }
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
            return true;
        }
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
            if (mname == "nil") {
                emitter.emit_loadnil(vars.push_var());
                return true;
            }
            const int res = vars.get_var(mname);
            if (res == -1) {
                const int f = vars.get_func(mname);
                if (f == -1) {
                    parser_throws(error_msg("identifier '" + mname + "' not found")) != nullptr;
                } else if (f == -2) {
                    parser_throws(error_msg("illegal expression with native function " + mname));
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
            eval_expr(dynamic_cast<UnaryExpr<UnaryOpType::MINUS> *>(expr)->expr.get(), emitter, vars);
            emitter.emit_neg(vars.last(), vars.last());
            break;
        case NodeType::BinaryPlus:
            simple_eval_binary<BinaryExpr<BinaryOpType::ADD>>(expr, emitter, vars);
            break;
        case NodeType::BinaryMod:
            simple_eval_binary<BinaryExpr<BinaryOpType::MOD>>(expr, emitter, vars);
            break;
        case NodeType::BinaryMul:
            simple_eval_binary<BinaryExpr<BinaryOpType::MUL>>(expr, emitter, vars);
            break;
        case NodeType::BinaryDiv:
            simple_eval_binary<BinaryExpr<BinaryOpType::DIV>>(expr, emitter, vars);
            break;
        case NodeType::BinaryMinus:
            simple_eval_binary<BinaryExpr<BinaryOpType::SUB>>(expr, emitter, vars);
            break;
        case NodeType::BinaryGR:
            simple_eval_binary<BinaryExpr<BinaryOpType::GR>>(expr, emitter, vars);
            break;
        case NodeType::BinaryLE:
            simple_eval_binary<BinaryExpr<BinaryOpType::LE>>(expr, emitter, vars);
            break;
        case NodeType::BinaryLS:
            simple_eval_binary<BinaryExpr<BinaryOpType::LS>>(expr, emitter, vars);
            break;
        case NodeType::BinaryGE:
            simple_eval_binary<BinaryExpr<BinaryOpType::GE>>(expr, emitter, vars);
            break;
        case NodeType::BinaryEQ:
            simple_eval_binary<BinaryExpr<BinaryOpType::EQ>>(expr, emitter, vars);
            break;
        case NodeType::BinaryNEQ:
            simple_eval_binary<BinaryExpr<BinaryOpType::NEQ>>(expr, emitter, vars);
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
