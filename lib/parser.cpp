#include "parser.h"
#include "lexer.h"
#include "misc.h"
#include "exceptions.h"
#include <fstream>
#include <map>
#include <cassert>
#include <array>
#include <memory>
#include <algorithm>
#include <format>
#include "var_manager.h"
#include "bytecode_emitter.h"
#include "expr_semantic.h"

namespace {
    using namespace ast;
    using namespace parser;

    BytecodeEmitter *emitter;
    VarManager vars;
    int lambda_count = 0;
    int jmp_uid = 0;//TODO: reset

    bool match(int token_type) {
        if (token_type != cur.token) return false;
        get_tok();
        return true;
    }

    bool test(int token_type) {
        return (token_type == cur.token);
    }

    unique_ptr<Node> parse_precedence(int prec);

    template<typename T>
    bool parse_param_list(int end_token, T check_add) {
        if (match(end_token)) return true;
        while (true) {
            if (!check_add()) return false;
            if (match(end_token)) return true;
            if (!match(TOKEN_COMMA)) return parser_throws(error_msg(", in parameter list")), false;
        }
    }

    //   cur.token = ->
    std::unique_ptr<FunctionSignature> parse_fn_params(std::string first = "") {
        auto res = std::make_unique<FunctionSignature>();
        res->name = "";
        if (!first.empty()) res->params.push_back(first);
        if (!parse_param_list(TOKEN_RPAREN, [&]() {
            if (cur.token != TOKEN_IDENTIFIER)
                return parser_throws(error_msg("identifier in function definition")), false;
            res->params.push_back(cur.identifier);
            get_tok();
            return true;
        }))
            return nullptr;
        return res;
    }

    //cur.token = "->"
    template<typename T = Node>
    std::unique_ptr<T> ifn_lambda_body(std::unique_ptr<Node> lhs) {
        if (lhs == nullptr || (lhs->get_type() != NodeType::FunctionSingature && lhs->get_type() != NodeType::Var)) {
            return parser_throws(error_msg("lambda arguments before ->"));
        }
        get_tok();

        //TODO: not supported because of the parse_block change for now
        return std::make_unique<FunctionDef>(std::move(lhs),
                                             match(TOKEN_LCURLY) ? parse_expression() : parse_expression());
//        return std::make_unique<FunctionDef>(std::move(lhs), match(TOKEN_LCURLY) ? parse_block() : parse_expression());
    }

    std::unique_ptr<Node> pfn_identifier() {
        auto res = std::make_unique<VarExpr>(cur.identifier);
        return (get_tok(), std::move(res));
    }

    std::unique_ptr<Node> pfn_number() {
        int64_t mul = 1;
        int start = 0;
        if (cur.identifier.size() > 1 && cur.identifier[0] == '-') {
            if (cur.identifier.size() > 20 ||
                cur.identifier.size() == 20 && cur.identifier > "-9223372036854775808")
                return parser_throws("Invalid number format: " + cur.identifier);
            mul = -1;
            start = 1;
        } else if (cur.identifier.size() > 19 ||
                   cur.identifier.size() == 19 && cur.identifier > "9223372036854775807") {
            return parser_throws("Invalid number format: " + cur.identifier);
        }
        int64_t res = 0;
        for (int i = start; i < cur.identifier.size(); ++i) {
            res *= 10ll;
            res += (int64_t) (cur.identifier[i] - '0') * mul;
        }
        get_tok();
        return std::make_unique<IntLitExpr>(res);
    }

    std::unique_ptr<Node> pfn_unary() {
        get_tok();
        if (test(TOKEN_INT_LIT)) {
            cur.identifier = "-" + cur.identifier;
            return pfn_number();
        }
        auto res = parse_precedence(PREC_UNARY);
        return res ? std::make_unique<UnaryExpr<UnaryOpType::MINUS>>(std::move(res)) : nullptr;
    }

    std::unique_ptr<Node> pfn_grouping();

    std::unique_ptr<Node> ifn_call(std::unique_ptr<Node> lhs) {
        std::unique_ptr<FunctionCall> f = std::make_unique<FunctionCall>(std::move(lhs));
        get_tok();
        if (!parse_param_list(TOKEN_RPAREN, [&]() {
            auto res = parse_expression();
            return !(res == nullptr) && (f->args.push_back(std::move(res)), true);
        }))
            return nullptr;
        return f;
    }

    std::unique_ptr<Node> ifn_arrayget(std::unique_ptr<Node> lhs) {
        get_tok();
        auto x = parse_expression();
        if (!x || !match(TOKEN_RBRACKET)) return parser_throws(error_msg("]"));
        return std::make_unique<ArrayGet>(std::move(lhs), std::move(x));
    }

    std::unique_ptr<Node> ifn_member(std::unique_ptr<Node> lhs) {
        get_tok();
        if (!match(TOKEN_IDENTIFIER)) return parser_throws(error_msg("member name after ."));
        return std::make_unique<MemberGet>(std::move(lhs), prv.identifier);
    }

    std::unique_ptr<Node> ifn_binary(std::unique_ptr<Node> lhs);

    std::unique_ptr<Node> pfn_string_lit() {
        get_tok();
        return std::make_unique<StringLitExpr>(prv.identifier);
    }
// clang-format off
// @formatter:off

    RuleInfo rules[] = {
    /* TOKEN_EOF */        {nullptr,        nullptr,    PREC_NONE},
    /* TOKEN_ADD */        {nullptr,        ifn_binary, PREC_ADD},
    /* TOKEN_SUB */        {pfn_unary,        ifn_binary, PREC_ADD},
    /* TOKEN_MUL */        {nullptr,        ifn_binary, PREC_FACTOR},
    /* TOKEN_DIV */        {nullptr,        ifn_binary, PREC_FACTOR},
    /* TOKEN_MOD */        {nullptr,        ifn_binary, PREC_FACTOR},
    /* TOKEN_IDENTIFIER */ {pfn_identifier, nullptr,    PREC_PRIMARY},
    /* TOKEN_INT_LIT */    {pfn_number,     nullptr,    PREC_PRIMARY},
    /* TOKEN_STR_LIT */    {pfn_string_lit,     nullptr,    PREC_PRIMARY},
    /* TOKEN_LBRACKET */   {nullptr,   ifn_arrayget,    PREC_CALL},
    /* TOKEN_RBRACKET */   {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_LPAREN */     {pfn_grouping,   ifn_call,    PREC_CALL},
    /* TOKEN_RPAREN */     {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_LCURLY */     {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_RCURLY */     {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_FN */         {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_ARROW */      {nullptr,   ifn_lambda_body,    PREC_ASSIGN, false},
    /* TOKEN_IF */         {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_ELSE */       {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_FOR */        {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_WHILE */      {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_RETURN */     {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_ASSIGN */     {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_EQ */         {nullptr,   ifn_binary,    PREC_EQ},
    /* TOKEN_AND */        {nullptr,   ifn_binary,    PREC_AND},
    /* TOKEN_OR */         {nullptr,   ifn_binary,    PREC_OR},
    /* TOKEN_LS */         {nullptr,   ifn_binary,    PREC_CMP},
    /* TOKEN_LE */         {nullptr,   ifn_binary,    PREC_CMP},
    /* TOKEN_GR */         {nullptr,   ifn_binary,    PREC_CMP},
    /* TOKEN_GE */         {nullptr,   ifn_binary,    PREC_CMP},
    /* TOKEN_SEMICOLON */  {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_COMMA */      {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_DOT */        {nullptr,   ifn_member,    PREC_CALL},
    /* TOKEN_UNKNOWN */    {nullptr,   nullptr,    PREC_NONE},
    };
// clang-format on
// @formatter:on

    std::unique_ptr<Node> pfn_grouping() {
        if (!match(TOKEN_LPAREN))
            return parser_throws(
                    error_msg("("));
        //either (), (<identifier>), (<identifier>, ...)
        if (cur.token == TOKEN_RPAREN) {
            get_tok();
            return std::make_unique<FunctionSignature>();
        }
        if (cur.token == TOKEN_IDENTIFIER) {
            const std::string first = cur.identifier;
            if (get_tok() == TOKEN_COMMA) {
                return get_tok(), parse_fn_params(first);
            }
            roll_back();
        }

        auto expr = parse_expression();
        if (cur.token != TOKEN_RPAREN)
            return parser_throws(
                    error_msg(")"));
        get_tok();
        return expr;
    }

    std::unique_ptr<Node> ifn_binary(std::unique_ptr<Node> lhs) {
        int op = cur.token;
        get_tok();
        auto rhs = parse_precedence(rules[op].precedence +
                                    rules[op].left_assoc);//because we want all operators to be left-assoc(if right-assoc needed just do not add 1)
        if (rhs == nullptr)
            return nullptr;
        switch (op) {
            case TOKEN_ADD:
                return std::make_unique<AddExpr>(std::move(lhs), std::move(rhs));
            case TOKEN_SUB:
                return std::make_unique<SubExpr>(std::move(lhs), std::move(rhs));
            case TOKEN_MUL:
                return std::make_unique<MulExpr>(std::move(lhs), std::move(rhs));
            case TOKEN_DIV:
                return std::make_unique<DivExpr>(std::move(lhs), std::move(rhs));
            case TOKEN_EQ:
                return std::make_unique<BinaryExpr<BinaryOpType::EQ>>(std::move(lhs), std::move(rhs));
            case TOKEN_LE:
                return std::make_unique<BinaryExpr<BinaryOpType::LE>>(std::move(lhs), std::move(rhs));
            case TOKEN_LS:
                return std::make_unique<BinaryExpr<BinaryOpType::LS>>(std::move(lhs), std::move(rhs));
            case TOKEN_GR:
                return std::make_unique<BinaryExpr<BinaryOpType::GR>>(std::move(lhs), std::move(rhs));
            case TOKEN_GE:
                return std::make_unique<BinaryExpr<BinaryOpType::GE>>(std::move(lhs), std::move(rhs));
            case TOKEN_AND:
                return std::make_unique<BinaryExpr<BinaryOpType::AND>>(std::move(lhs), std::move(rhs));
            case TOKEN_OR:
                return std::make_unique<BinaryExpr<BinaryOpType::OR>>(std::move(lhs), std::move(rhs));
            case TOKEN_MOD:
                return std::make_unique<BinaryExpr<BinaryOpType::MOD>>(std::move(lhs), std::move(rhs));
            default:
                return parser_throws("Unknown binary operator | or internal error");
        }
    }

    unique_ptr<Node> parse_precedence(int prec) {
        if (rules[cur.token].prefix == nullptr)
            return parser_throws(error_msg("identifier"));
        auto lhs = rules[cur.token].prefix();
        int op = cur.token;
        while (lhs && prec <= rules[op].precedence) {
            if (rules[op].infix == nullptr)
                return parser_throws(error_msg("operator"));
            lhs = rules[op].infix(std::move(lhs));
            op = cur.token;
        }
        return lhs;
    }
}
namespace parser {

    unique_ptr<Node> parse_expression() {
        return parse_precedence(PREC_ASSIGN);
    }


    //Note: doesn't close the scope
    void parse_block() {
        vars.new_scope();
        while (true) {
            switch (cur.token) {
                case TOKEN_RCURLY:
                    get_tok();
                    vars.close_scope();
                    return;
                case TOKEN_EOF:
                    parser_throws("expected } but found EOF");
                    return;
                default:
                    parse_statement();
                    if (is_panic()) return;
            }
        }
    }

    //cur_tok = '('
    void if_statement() {
        if (!match(TOKEN_LPAREN)) {
            parser_throws(error_msg("( after if keyword"));
            return;
        }
        if (!epush(parse_expression())) return;
        if (!match(TOKEN_RPAREN)) {
            parser_throws(error_msg("closing ) in if statement"));
            return;
        }
        const int cur_id1 = jmp_uid++;
        emitter->jmpf_label(vars.pop_var(), cur_id1);

        if (match(TOKEN_LCURLY)) {
            parse_block();
        } else parse_statement(); //TODO: why here was parse_expression????


        if (is_panic()) return;
        if (match(TOKEN_ELSE)) {
            if (match(TOKEN_IF)) {
                throw std::runtime_error("not supported for now");
                //if_statement();
            } else {
                const int cur_id2 = jmp_uid++;
                emitter->jmp_label(cur_id2);
                emitter->label(cur_id1);
                if (match(TOKEN_LCURLY)) {
                    parse_block();
                } else parse_statement();
                emitter->label(cur_id2);
                //elsebody = match(TOKEN_LCURLY) ? parse_block() : parse_statement();
            }
        }
        emitter->label(cur_id1);
    }

    std::unique_ptr<ast::Node> parse_expr_sc() {
        auto res = parse_expression();
        if (!match(TOKEN_SEMICOLON)) return parser_throws(error_msg("; after expression"));
        return res;
    }

//    template<typename F, typename T>
//    std::unique_ptr<Node> parse_typical_statement(F cond, T make_res, std::string err1, std::string err2) {
//        if (!match(TOKEN_LPAREN)) return parser_throws(error_msg(err1));
//        auto res = cond();
//        if (res == nullptr) return nullptr;
//
//        if (!match(TOKEN_RPAREN)) return parser_throws(error_msg("closing ) in while statement"));
//        auto body = match(TOKEN_LCURLY) ? parse_block() : parse_statement();
//        if (body == nullptr) return nullptr;
//        return make_res(std::move(res), std::move(body));
//    }

    void parse_while() {
        throw std::runtime_error("TODO");
//        return parse_typical_statement(parse_expression, [](std::unique_ptr<Node> expr, std::unique_ptr<Node> body) {
//            return std::make_unique<WhileStmt>(std::move(expr), std::move(body));
//        }, "( after while", ") in while statement");
    }

    void parse_for() {
        throw std::runtime_error("TODO");
//        return parse_typical_statement([]() -> std::unique_ptr<ForStmt> {
//            std::unique_ptr<ForStmt> s = std::make_unique<ForStmt>();
//            s->init = parse_expr_sc();
//            if (s->init == nullptr) return nullptr;
//            s->cond = parse_expr_sc();
//            if (s->cond == nullptr) return nullptr;
//            s->inc = parse_expression();
//            if (s->inc == nullptr) return nullptr;
//            return s;
//        }, [](std::unique_ptr<Node> expr, std::unique_ptr<Node> body) {
//            return std::make_unique<WhileStmt>(std::move(expr), std::move(body));
//        }, "( after for", ") in for statement");
    }


    //make sure that cur.token != TOKEN_EOF && cur.token != TOKEN_RCURLY on call
    void parse_statement() {
        if (match(TOKEN_IF)) if_statement();
        else if (match(TOKEN_FOR)) parse_for();
        else if (match(TOKEN_WHILE)) parse_while();
        else if (match(TOKEN_RETURN)) parse_return();
        else {
            auto lhs = parse_expression();
            if (lhs == nullptr) return;
            if (match(TOKEN_SEMICOLON)) { return; }
            if (!match(TOKEN_ASSIGN)) {
                parser_throws(error_msg("; after expression"));
                return;
            }
            if (!epush(parse_expression())) return;
            if (!match(TOKEN_SEMICOLON)) { return; }
            if (lhs->get_type() != ast::NodeType::Var) {
                parser_throws(error_msg("todo16"));
                return;
            }
            const std::string &m_name = dynamic_cast<VarExpr *>(lhs.get())->name;
            const int res = vars.get_var(m_name);
            //if variable not found, then we create new at the same place and it has almost no scope
            if (res == -1) {
                vars.pop_var();
                vars.push_var(m_name);
                return;
            }
            emitter->emit_move(res, vars.pop_var());
        }
    }

    ast::Program parse_program() {
        Program res;
        while (cur.token != TOKEN_EOF) {
            bool had_errors = is_panic();
            if (is_panic()) {
                panic();
            }
            if ((had_errors ? prv.token : cur.token) == TOKEN_FN) {
                if (!had_errors) get_tok();
                parse_function();
//                res.declarations.push_back(parse_function());
            } else parser_throws(error_msg("function definition"));
        }
        emitter->resolve();
        res.instructions = std::move(emitter->code);
        delete emitter;
        return res;
    }

    //if anonymous: cur = first argument
    //otherwise: cur = function name
    void parse_function() {
        if (!match(TOKEN_IDENTIFIER)) {
            parser_throws(error_msg("function name"));
            return;
        }
        std::string name = std::move(prv.identifier);
        if (!match(TOKEN_LPAREN)) {
            parser_throws(error_msg("("));
            return;
        }
        auto header = parse_fn_params();
        if (header == nullptr) { return; }
        if (!match(TOKEN_LCURLY)) {
            parser_throws(error_msg("{ in function body"));
            return;
        }
        emitter->begin_func(name, header->params.size());
        for (auto &cur: header->params) {
            auto it = vars.get_var(cur);
            if (it != -1) {
                parser_throws(std::format("illegal argument names, argument {} already exsists", cur));
                return;
            }
            vars.push_var(cur);
        }
        parse_block();
        //add return in case control flow leaks
        emitter->emit_loadnil(vars.push_var());
        emitter->emit_return(vars.pop_var());
        emitter->end_func();
    }


    void init_parser(std::istream &in, BytecodeEmitter *emit) {
        init_lexer(in);
        emitter = emit;
        init_exceptions();
    }

    bool epush(std::unique_ptr<ast::Node> expr) {
        if (expr == nullptr) return false;
        return parser::eval_expr(parser::check_expr(std::move(expr), *emitter, vars), *emitter, vars);
    }

    void parse_return() {
        epush(parse_expr_sc());
        emitter->emit_return(vars.pop_var());
    }

}
//TODO: panic on error and output many errors