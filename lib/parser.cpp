#include "parser.h"
#include "lexer.h"
#include "misc.h"
#include "exceptions.h"
#include <fstream>
#include <map>
#include <cassert>
#include <array>
#include <memory>

namespace {
    using namespace ast;
    using namespace parser;

    bool match(int token_type, int token_info = ANY_TOKEN_EXPECTED) {
        if (token_type != cur.token) return false;
        get_tok(token_info);
        return true;
    }

    unique_ptr<Node> parse_precedence(int prec);

    template<int last_token_info = ANY_TOKEN_EXPECTED, typename T>
    bool parse_param_list(int end_token, T check_add) {
        if (match(end_token, last_token_info)) return true;
        while (true) {
            if (!check_add()) return false;
            if (match(end_token, last_token_info)) return true;
            if (!match(TOKEN_COMMA)) return parser_throws(error_msg(", in parameter list")), false;
        }
    }

    //   cur.token = ->
    std::unique_ptr<FunctionSignature> parse_fn_params(std::string first = "") {
        auto res = std::make_unique<FunctionSignature>();
        res->name = "";
        if (!first.empty()) res->params.push_back(first);
        if (!parse_param_list<OPERATOR_EXPECTED>(TOKEN_RPAREN, [&]() {
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
        return std::make_unique<FunctionDef>(std::move(lhs), match(TOKEN_LCURLY) ? parse_block() : parse_expression());
    }

    std::unique_ptr<Node> pfn_identifier() {
        auto res = std::make_unique<VarExpr>(cur.identifier);
        return (get_tok(OPERATOR_EXPECTED), std::move(res));
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
        get_tok(OPERATOR_EXPECTED);
        return std::make_unique<IntLitExpr>(res);
    }

    std::unique_ptr<Node> pfn_unary() {
        get_tok();
        auto res = parse_precedence(PREC_UNARY);
        return res ? std::make_unique<UnaryExpr<UnaryOpType::MINUS>>(std::move(res)) : nullptr;
    }

    std::unique_ptr<Node> pfn_grouping();

    std::unique_ptr<Node> ifn_call(std::unique_ptr<Node> lhs) {
        std::unique_ptr<FunctionCall> f = std::make_unique<FunctionCall>(std::move(lhs));
        get_tok();
        if (!parse_param_list<OPERATOR_EXPECTED>(TOKEN_RPAREN, [&]() {
            auto res = parse_expression();
            return !(res == nullptr) && (f->args.push_back(std::move(res)), true);
        }))
            return nullptr;
        return f;
    }

    std::unique_ptr<Node> ifn_arrayget(std::unique_ptr<Node> lhs) {
        get_tok();
        auto x = parse_expression();
        if (!x || !match(TOKEN_RBRACKET, OPERATOR_EXPECTED)) return parser_throws(error_msg("]"));
        return std::make_unique<ArrayGet>(std::move(lhs), std::move(x));
    }

    std::unique_ptr<Node> ifn_member(std::unique_ptr<Node> lhs) {
        get_tok();
        if (!match(TOKEN_IDENTIFIER, OPERATOR_EXPECTED)) return parser_throws(error_msg("member name after ."));
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
    /* TOKEN_ASSIGN */     {nullptr,   ifn_binary,    PREC_ASSIGN, false},
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
            get_tok(OPERATOR_EXPECTED);
            return std::make_unique<FunctionSignature>();
        }
        if (cur.token == TOKEN_IDENTIFIER) {
            const std::string first = cur.identifier;
            if (get_tok(OPERATOR_EXPECTED) == TOKEN_COMMA) {
                return get_tok(), parse_fn_params(first);
            }
            roll_back();
        }

        auto expr = parse_expression();
        if (cur.token != TOKEN_RPAREN)
            return parser_throws(
                    error_msg(")"));
        get_tok(OPERATOR_EXPECTED);
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
            case TOKEN_ASSIGN:
                return std::make_unique<BinaryExpr<BinaryOpType::ASSIGN>>(std::move(lhs), std::move(rhs));
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


    unique_ptr<ast::Block> parse_block() {
        std::unique_ptr<Block> res = std::make_unique<Block>();
        while (true) {
            switch (cur.token) {
                case TOKEN_RCURLY:
                    get_tok();
                    return res;
                case TOKEN_EOF:
                    return parser_throws("expected } but found EOF");
                default:
                    res->lines.push_back(parse_statement());
                    if (res->lines.back() == nullptr) return nullptr;
            }
        }
    }

    //cur_tok = '('
    std::unique_ptr<ast::Node> if_statement() {
        if (!match(TOKEN_LPAREN)) return parser_throws(error_msg("( after if keyword"));
        auto res = parse_expression();
        if (res == nullptr) return nullptr;
        if (!match(TOKEN_RPAREN)) return parser_throws(error_msg("closing ) in if statement"));
        auto body = match(TOKEN_LCURLY) ? parse_block() : parse_statement();
        if (body == nullptr) return nullptr;
        std::unique_ptr<ast::Node> elsebody = nullptr;
        if (match(TOKEN_ELSE)) {
            if (match(TOKEN_IF)) {
                elsebody = if_statement();
            } else {
                elsebody = match(TOKEN_LCURLY) ? parse_block() : parse_statement();
            }
        }
        return std::make_unique<IfStmt>(std::move(res), std::move(body), std::move(elsebody));
    }

    std::unique_ptr<ast::Node> parse_expr_sc() {
        auto res = parse_expression();
        if (!match(TOKEN_SEMICOLON)) return parser_throws(error_msg("; after expression"));
        return res;
    }

    template<typename F, typename T>
    std::unique_ptr<Node> parse_typical_statement(F cond, T make_res, std::string err1, std::string err2) {
        if (!match(TOKEN_LPAREN)) return parser_throws(error_msg(err1));
        auto res = cond();
        if (res == nullptr) return nullptr;
        if (!match(TOKEN_RPAREN)) return parser_throws(error_msg("closing ) in while statement"));
        auto body = match(TOKEN_LCURLY) ? parse_block() : parse_statement();
        if (body == nullptr) return nullptr;
        return make_res(std::move(res), std::move(body));
    }

    std::unique_ptr<Node> parse_while() {
        return parse_typical_statement(parse_expression, [](std::unique_ptr<Node> expr, std::unique_ptr<Node> body) {
            return std::make_unique<WhileStmt>(std::move(expr), std::move(body));
        }, "( after while", ") in while statement");
    }

    std::unique_ptr<Node> parse_for() {
        return parse_typical_statement([]() -> std::unique_ptr<ForStmt> {
            std::unique_ptr<ForStmt> s = std::make_unique<ForStmt>();
            s->init = parse_expr_sc();
            if (s->init == nullptr) return nullptr;
            s->cond = parse_expr_sc();
            if (s->cond == nullptr) return nullptr;
            s->inc = parse_expression();
            if (s->inc == nullptr) return nullptr;
            return s;
        }, [](std::unique_ptr<Node> expr, std::unique_ptr<Node> body) {
            return std::make_unique<WhileStmt>(std::move(expr), std::move(body));
        }, "( after for", ") in for statement");
    }

    //make sure that cur.token != TOKEN_EOF && cur.token != TOKEN_RCURLY on call
    unique_ptr<ast::Node> parse_statement() {
        if (match(TOKEN_IF)) return if_statement();
        if (match(TOKEN_FOR)) return parse_for();
        if (match(TOKEN_WHILE)) return parse_while();
        if (match(TOKEN_RETURN)) return std::make_unique<ReturnStmt>(parse_expr_sc());
        return parse_expr_sc();
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
                res.declarations.push_back(parse_function());
            } else parser_throws(error_msg("function definition"));
        }
        return res;
    }

    //if anonymous: cur = first argument
    //otherwise: cur = function name
    std::unique_ptr<FunctionDef> parse_function() {
        if (!match(TOKEN_IDENTIFIER)) return parser_throws(error_msg("function name"));
        std::string name = std::move(prv.identifier);
        if (!match(TOKEN_LPAREN)) return parser_throws(error_msg("("));
        auto header = parse_fn_params();
        if (header == nullptr) return nullptr;
        header->name = name;
        if (!match(TOKEN_LCURLY)) return parser_throws(error_msg("{ in function body"));
        return std::make_unique<FunctionDef>(std::move(header), parse_block());
    }


    void init_parser(std::istream &in) {
        init_lexer(in);
        init_exceptions();
    }

}
//TODO: panic on error and output many errors