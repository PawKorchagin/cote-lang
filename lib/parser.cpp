#include "parser.h"
#include "mish.h"
#include <fstream>
#include <map>
#include <cassert>
#include <array>

namespace {
    using namespace ast;
    using namespace parser;

    //wrapper for keeping cursor position and error messages
    struct SimpleStream {
        SimpleStream(std::istream &in) : in(in) {}

        int get() {
            int c = in.get();
            if (c == '\r' && in.peek() == '\n') return get();
            cnt++;
            if (c == '\r' || c == '\n') {
                c = '\n';
                cnt = 1;
                lines++;
            }
            return c;
        }

        int plines() { return lines; }

        int pcnt() { return cnt; }

    private:
        int lines = 1;
        int cnt = 0;
        std::istream &in;
    };

    struct TokenData {
        std::string identifier = "";
        int token = 0;
        int lines = 0;
        int cnt = 0;
    };
    TokenData cur, prv;
    std::vector<std::string> error_log;
    int cur_char = ' ';
    SimpleStream in(NULL_STREAM);
    void *m_stream_ptr = &in;

    bool panic_mode = false;

    std::nullptr_t parser_throws(const std::string &message) {
        if (panic_mode) return nullptr;
        panic_mode = true;

        error_log.push_back(
                std::to_string(cur.lines) + ":" + std::to_string(cur.cnt) + ": " + message);
        return nullptr;
    }

    std::string error_msg(const std::string &text) {
        return "expected " + text + " but found " + token_to_string(static_cast<TokenInfo>(cur.token), cur.identifier);
    }

//-----------------LEXER----------------------

    int helper_return_char(int TOKEN) {
        cur_char = in.get();
        return cur.token = TOKEN;
    }

    int tok_number() {
        cur.identifier.clear();
        int expected_size = int(cur_char == '-') + 1;
        do {
            cur.identifier.push_back(static_cast<char>(cur_char));
            cur_char = in.get();
        } while (isdigit(cur_char));
        if (cur.identifier.size() < expected_size) {
            if (cur_char == '-') return parser_throws("illegal token --"), cur.token = TOKEN_UNKNOWN;
            return cur.token = TOKEN_SUB;
        }
        return cur.token = TOKEN_INT_LIT;
    }

    int parse_token2(char second, int res1, int res2) {
        cur_char = in.get();
        if (cur_char == second) return helper_return_char(res2);
        return cur.token = res1;
    }

    int tok_identifier() {
        cur.identifier.clear();
        do {
            cur.identifier.push_back(static_cast<char>(cur_char));
            cur_char = in.get();
        } while (isalnum(cur_char) || cur_char == '_');
        if (cur.identifier == "fn") return cur.token = TOKEN_FN;
        if (cur.identifier == "if") return cur.token = TOKEN_IF;
        if (cur.identifier == "while") return cur.token = TOKEN_WHILE;
        if (cur.identifier == "else") return cur.token = TOKEN_ELSE;
        return cur.token = TOKEN_IDENTIFIER;
    }

    int get_tok(int token_info = ANY_TOKEN_EXPECTED) {
        std::swap(prv, cur);
        cur.lines = in.plines();
        cur.cnt = in.pcnt();
        while (isspace(cur_char)) cur_char = in.get();
        if (isdigit(cur_char) || (token_info & VALUE_EXPECTED) && cur_char == '-')
            return tok_number();
        if (isalpha(cur_char) || cur_char == '_')
            return tok_identifier();
        if (cur_char == '(') return helper_return_char(TOKEN_LPAREN);
        if (cur_char == ')') return helper_return_char(TOKEN_RPAREN);
        if (cur_char == '}') return helper_return_char(TOKEN_RCURLY);
        if (cur_char == '{') return helper_return_char(TOKEN_LCURLY);
        if (cur_char == -1) return cur.token = TOKEN_EOF;
        switch (cur_char) {
            case ',':
                return helper_return_char(TOKEN_COMMA);
            case '+':
                return helper_return_char(TOKEN_ADD);
            case '-':
                cur_char = in.get();
                if (cur_char == '-')
                    return parser_throws("illegal token --"), cur.token = TOKEN_UNKNOWN;
                return cur.token = TOKEN_SUB;
            case '*':
                return helper_return_char(TOKEN_MUL);
            case '/':
                cur_char = in.get();
                if (cur_char == '/') {
                    while (cur_char != '\n') cur_char = in.get();
                    return get_tok(token_info);
                }
                return cur.token = TOKEN_DIV;
            case '=':
                return parse_token2('=', TOKEN_ASSIGN, TOKEN_EQ);
            case ';':
                return helper_return_char(TOKEN_SEMICOLON);
            case '<':
                return parse_token2('=', TOKEN_LS, TOKEN_LE);
            case '>':
                return parse_token2('=', TOKEN_GR, TOKEN_GE);
            default:
                return parser_throws("unknown token " + std::to_string(char(cur_char))), helper_return_char(
                        TOKEN_UNKNOWN);

        }
    }

//-----------------LEXER----------------------
// ---------------ERROR SYSTEM-----------------


//stops at ; or declaration or statement
    void panic() {
        while (true) {
            switch (cur.token) {
                case TOKEN_EOF:
                case TOKEN_FN:
                    panic_mode = false;
                    return;
                default:
                    get_tok();
            }
        }
        //TODO
    }

// ---------------ERROR SYSTEM-----------------

    bool match(int token_type) {
        if (token_type != cur.token) return false;
        get_tok();
        return true;
    }

    unique_ptr<Node> parse_precedence(int prec);

    template<typename T>
    bool parse_param_list(T check_add) {
        if (match(TOKEN_RPAREN)) return true;
        while (true) {
            if (!check_add()) return false;
            if (match(TOKEN_RPAREN)) return true;
            if (!match(TOKEN_COMMA)) return parser_throws(error_msg(", in parameter list")), false;
        }
    }

    std::unique_ptr<Node> pfn_identifier() {
        auto res = std::make_unique<VarExpr>(cur.identifier);
        return get_tok(OPERATOR_EXPECTED), std::move(res);
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

    std::unique_ptr<Node> pfn_grouping() {
        if (!match(TOKEN_LPAREN))
            return parser_throws(
                    error_msg("("));
        auto expr = parse_expression();
        if (cur.token != TOKEN_RPAREN)
            return parser_throws(
                    error_msg(")"));
        get_tok(OPERATOR_EXPECTED);
        return expr;
    }

    std::unique_ptr<Node> pfn_unary() {
        get_tok();
        auto res = parse_precedence(PREC_UNARY);
        return res ? std::make_unique<UnaryExpr<UnaryOpType::MINUS>>(std::move(res)) : nullptr;
    }


    std::unique_ptr<Node> pfn_lambda() {
        get_tok();
        return parse_function(true);
    }

    std::unique_ptr<Node> ifn_call(std::unique_ptr<Node> lhs) {
        if (prv.token != TOKEN_IDENTIFIER) return parser_throws(error_msg("function name"));
        std::unique_ptr<FunctionCall> f = std::make_unique<FunctionCall>(prv.identifier);
        get_tok();
        if (!parse_param_list([&]() {
            auto res = parse_expression();
            return !(res == nullptr) && (f->args.push_back(std::move(res)), true);
        }))
            return nullptr;
        return f;
    }

    std::unique_ptr<Node> ifn_binary(std::unique_ptr<Node> lhs);

// clang-format off
// @formatter:off

    RuleInfo rules[] = {
    /* TOKEN_EOF */         {nullptr,        nullptr,    PREC_NONE},
    /* TOKEN_ADD */         {nullptr,        ifn_binary, PREC_ADD},
    /* TOKEN_SUB */         {pfn_unary,        ifn_binary, PREC_ADD},
    /* TOKEN_MUL */         {nullptr,        ifn_binary, PREC_FACTOR},
    /* TOKEN_DIV */         {nullptr,        ifn_binary, PREC_FACTOR},
    /* TOKEN_IDENTIFIER */  {pfn_identifier, nullptr,    PREC_PRIMARY},
    /* TOKEN_INT_LIT */     {pfn_number,     nullptr,    PREC_PRIMARY},
    /* TOKEN_LPAREN */          {pfn_grouping,   ifn_call,    PREC_CALL},
    /* TOKEN_RPAREN */      {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_LCURLY */      {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_RCURLY */     {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_FN */         {pfn_lambda,   nullptr,    PREC_NONE},
    /* TOKEN_IF */         {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_ELSE */       {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_WHILE */      {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_ASSIGN */     {nullptr,   ifn_binary,    PREC_ASSIGN},
    /* TOKEN_EQ */         {nullptr,   ifn_binary,    PREC_EQ},
    /* TOKEN_LS */         {nullptr,   ifn_binary,    PREC_CMP},
    /* TOKEN_LE */         {nullptr,   ifn_binary,    PREC_CMP},
    /* TOKEN_GR */         {nullptr,   ifn_binary,    PREC_CMP},
    /* TOKEN_GE */         {nullptr,   ifn_binary,    PREC_CMP},
    /* TOKEN_SEMICOLON */  {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_COMMA */      {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_UNKNOWN */    {nullptr,   nullptr,    PREC_NONE},
    };
// clang-format on
// @formatter:on

    std::unique_ptr<Node> ifn_binary(std::unique_ptr<Node> lhs) {
        int op = cur.token;
        get_tok();
        auto rhs = parse_precedence(rules[op].precedence +
                                    1);//because we want all operators to be left-assoc(if right-assoc needed just do not add 1)
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

    void init_parser(std::istream &input_stream) {
        error_log.clear();
        in.~SimpleStream();
        panic_mode = false;
        new(m_stream_ptr) SimpleStream(input_stream);
        cur_char = ' ';
        get_tok();
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
        if (!match(TOKEN_LCURLY)) return parser_throws(error_msg("{"));
        auto body = parse_block();
        if (body == nullptr) return nullptr;
        std::unique_ptr<ast::Node> elsebody = nullptr;
        if (match(TOKEN_ELSE)) {
            if (match(TOKEN_IF)) {
                elsebody = if_statement();
            } else {
                if (!match(TOKEN_LCURLY)) return parser_throws(error_msg("{ after else"));
                elsebody = parse_block();
            }
        }
        return std::make_unique<IfStmt>(std::move(res), std::move(body), std::move(elsebody));
    }

    //make sure that cur.token != TOKEN_EOF && cur.token != TOKEN_RCURLY on call
    unique_ptr<ast::Node> parse_statement() {
        if (match(TOKEN_IF)) return if_statement();
        if (match(TOKEN_WHILE)) {
            if (!match(TOKEN_LPAREN)) return parser_throws(error_msg("( after if keyword"));
            auto res = parse_expression();
            if (res == nullptr) return nullptr;
            if (!match(TOKEN_RPAREN)) return parser_throws(error_msg("closing ) in if statement"));
            if (!match(TOKEN_LCURLY)) return parser_throws(error_msg("{"));
            auto body = parse_block();
            if (body == nullptr) return nullptr;
            return std::make_unique<WhileStmt>(std::move(res), std::move(body));
        }
        auto res = parse_expression();
        if (!match(TOKEN_SEMICOLON)) return parser_throws(error_msg("; after expression"));
        return res;
    }

    ast::Program parse_program() {
        Program res;
        while (cur.token != TOKEN_EOF) {
            if (match(TOKEN_FN)) {
                res.declarations.push_back(parse_function());
            } else {
                parser_throws(error_msg("function definition"));
                panic_mode = true;
            }
            if (panic_mode) panic();
        }
        return res;
    }

    unique_ptr<ast::FunctionDef> parse_function(bool anonymous) {
        std::unique_ptr<FunctionDef> f = std::unique_ptr<FunctionDef>(new FunctionDef());
        FunctionSignature fsig;
        if (!anonymous) {
            if (!match(TOKEN_IDENTIFIER)) {
                return parser_throws(error_msg("function name"));
            }
            fsig.name = std::move(cur.identifier);
            cur.identifier = "";
        }
        if (!match(TOKEN_LPAREN)) return parser_throws(error_msg("("));
        if (!parse_param_list([&]() {
            if (cur.token != TOKEN_IDENTIFIER)
                return parser_throws(error_msg("identifier in function definition")), false;
            fsig.params.push_back(cur.identifier);
            get_tok();
            return true;
        }))
            return nullptr;
        if (!match(TOKEN_LCURLY)) return parser_throws(error_msg("{"));
        f->signature = std::move(fsig);
        f->block = parse_block();
        return f;
    }

    std::string token_to_string(TokenInfo tok, std::string temp_data) {
        switch (tok) {
            case TOKEN_EOF:
                return "EOF";
            case TOKEN_ADD:
                return "+";
            case TOKEN_MUL:
                return "*";
            case TOKEN_SUB:
                return "-";
            case TOKEN_DIV:
                return "/";
            case TOKEN_LPAREN:
                return "(";
            case TOKEN_RPAREN:
                return ")";
            case TOKEN_LCURLY:
                return "{";
            case TOKEN_RCURLY:
                return "}";
            case TOKEN_FN:
                return "fn";
            case TOKEN_UNKNOWN:
                return std::to_string(char(tok));
            case TOKEN_IDENTIFIER:
            case TOKEN_INT_LIT:
                return temp_data;
            case TOKEN_IF:
                return "if";
            case TOKEN_WHILE:
                return "while";
            case TOKEN_ASSIGN:
                return "=";
            case TOKEN_EQ:
                return "==";
            case TOKEN_SEMICOLON:
                return ";";
            case TOKEN_ELSE:
                return "else";
            case TOKEN_LS:
                return "<";
            case TOKEN_LE:
                return "<=";
            case TOKEN_GR:
                return ">";
            case TOKEN_GE:
                return ">=";
            case TOKEN_COMMA:
                return ",";
        }
        throw std::runtime_error("token_to_string failed - internal error");
    }


    std::vector<std::string> get_errors() {
        return error_log;
    }
}
//TODO: panic on error and output many errors