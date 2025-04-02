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

        void update_pos() {
            saved_lines = lines;
            saved_cnt = cnt;
        }

        int plines() { return saved_lines; }
        int pcnt() { return saved_cnt; }
    private:
        int lines = 1;
        int cnt = 0;
        int saved_lines = 0;
        int saved_cnt = 0;
        std::istream &in;
    };

    std::string temp_identifier;
    std::vector<std::string> error_log;
    int cur_char = ' ';
    int cur_token = 0;
    SimpleStream in(NULL_STREAM);
    void *m_stream_ptr = &in;

    bool panic_mode = false;

    std::nullptr_t parser_throws(const std::string &message) {
        if (panic_mode) return nullptr;
        panic_mode = true;

        error_log.push_back(
                std::to_string(in.plines()) + ":" + std::to_string(in.pcnt()) + ": " + message);
        return nullptr;
    }


//-----------------LEXER----------------------

    int helper_return_char(int TOKEN) {
        cur_char = in.get();
        return cur_token = TOKEN;
    }

    int tok_number() {
        temp_identifier.clear();
        int expected_size = int(cur_char == '-') + 1;
        do {
            temp_identifier.push_back(static_cast<char>(cur_char));
            cur_char = in.get();
        } while (isdigit(cur_char));
        if (temp_identifier.size() < expected_size) {
            if (cur_char == '-') return parser_throws("illegal token --"), cur_token = TOKEN_UNKNOWN;
            return cur_token = TOKEN_SUB;
        }
        return cur_token = TOKEN_INT_LIT;
    }

    int tok_identifier() {
        temp_identifier.clear();
        do {
            temp_identifier.push_back(static_cast<char>(cur_char));
            cur_char = in.get();
        } while (isalnum(cur_char) || cur_char == '_');
        if (temp_identifier == "fn") return cur_token = TOKEN_FN;
        return cur_token = TOKEN_IDENTIFIER;
    }

    int get_tok(int token_info = ANY_TOKEN_EXPECTED) {
        in.update_pos();
        while (isspace(cur_char)) cur_char = in.get();
        if (isdigit(cur_char) || (token_info & VALUE_EXPECTED) && cur_char == '-')
            return tok_number();
        if (isalpha(cur_char) || cur_char == '_')
            return tok_identifier();
        if (cur_char == '(') return helper_return_char(TOKEN_LPAREN);
        if (cur_char == ')') return helper_return_char(TOKEN_RPAREN);
        if (cur_char == '}') return helper_return_char(TOKEN_RCURLY);
        if (cur_char == '{') return helper_return_char(TOKEN_LCURLY);
        if (cur_char == -1) return cur_token = TOKEN_EOF;
        switch (cur_char) {
            case '+':
                return helper_return_char(TOKEN_ADD);
            case '-':
                cur_char = in.get();
                if (cur_char == '-')
                    return parser_throws("illegal token --"), cur_token = TOKEN_UNKNOWN;
                return cur_token = TOKEN_SUB;
            case '*':
                return helper_return_char(TOKEN_MUL);
            case '/':
                cur_char = in.get();
                if (cur_char == '/') {
                    while (cur_char != '\n') cur_char = in.get();
                    return get_tok(token_info);
                }
                return cur_token = TOKEN_DIV;
            case '=':
                return helper_return_char(TOKEN_ASSIGN);
            case ';':
                return helper_return_char(TOKEN_SEMICOLON);
            default:
                return parser_throws("unknown token " + std::to_string(char(cur_char))), cur_token = TOKEN_UNKNOWN;

        }
    }

//-----------------LEXER----------------------
// ---------------ERROR SYSTEM-----------------


//stops at ; or declaration or statement
    void panic() {
        while (true) {
            switch (cur_token) {
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
        if (token_type != cur_token) return false;
        get_tok();
        return true;
    }

    unique_ptr<Node> parse_precedence(int prec);

    std::unique_ptr<Node> pfn_identifier() {
        auto res = std::make_unique<VarExpr>(temp_identifier);
        return get_tok(OPERATOR_EXPECTED), std::move(res);
    }

    std::unique_ptr<Node> pfn_number() {
        int64_t mul = 1;
        int start = 0;
        if (temp_identifier.size() > 1 && temp_identifier[0] == '-') {
            if (temp_identifier.size() > 20 ||
                temp_identifier.size() == 20 && temp_identifier > "-9223372036854775808")
                return parser_throws("Invalid number format: " + temp_identifier);
            mul = -1;
            start = 1;
        } else if (temp_identifier.size() > 19 ||
                   temp_identifier.size() == 19 && temp_identifier > "9223372036854775807") {
            return parser_throws("Invalid number format: " + temp_identifier);
        }
        int64_t res = 0;
        for (int i = start; i < temp_identifier.size(); ++i) {
            res *= 10ll;
            res += (int64_t) (temp_identifier[i] - '0') * mul;
        }
        get_tok(OPERATOR_EXPECTED);
        return std::make_unique<IntLitExpr>(res);
    }

    std::unique_ptr<Node> pfn_grouping() {
        if (!match(TOKEN_LPAREN))
            return parser_throws(
                    "expected ( but found " + token_to_string(static_cast<TokenInfo>(cur_token), temp_identifier));
        auto expr = parse_expression();
        if (cur_token != TOKEN_RPAREN)
            return parser_throws(
                    "expected ) but found " + token_to_string(static_cast<TokenInfo>(cur_token), temp_identifier));
        get_tok(OPERATOR_EXPECTED);
        return expr;
    }

    std::unique_ptr<Node> pfn_unary() {
        get_tok();
        auto res = parse_precedence(PREC_UNARY);
        return res ? std::make_unique<UnaryExpr<UnaryOpType::MINUS>>(std::move(res)) : nullptr;
    }

    std::unique_ptr<Node> ifn_binary(std::unique_ptr<Node> lhs, int op);

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
    /* TOKEN_LPAREN */      {pfn_grouping,   nullptr,    PREC_CALL},
    /* TOKEN_RPAREN */      {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_LCURLY */      {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_RCURLY */     {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_FN */         {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_ASSIGN */     {nullptr,   ifn_binary,    PREC_ASSIGN},
    /* TOKEN_SEMICOLON */  {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_UNKNOWN */    {nullptr,   nullptr,    PREC_NONE},
    };
// clang-format on
// @formatter:on

    std::unique_ptr<Node> ifn_binary(std::unique_ptr<Node> lhs, int op) {
        auto rhs = parse_precedence(rules[op].precedence +
                                    1);//because we want all operators to be left-assoc(if right-assoc needed just do not add 1)
        if (rhs == nullptr)
            return parser_throws(
                    "expected value but found " + token_to_string(static_cast<TokenInfo>(cur_token), temp_identifier));
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
            default:
                return parser_throws("Unknown binary operator | or internal error");
        }
    }

    unique_ptr<Node> parse_precedence(int prec) {
        if (rules[cur_token].prefix == nullptr)
            return parser_throws("expected identifier but found " +
                                 token_to_string(static_cast<TokenInfo>(cur_token), temp_identifier));
        auto lhs = rules[cur_token].prefix();
        int op = cur_token;
        while (lhs && prec <= rules[op].precedence) {
            if (rules[op].infix == nullptr)
                return parser_throws("expected operator but found " + token_to_string(static_cast<TokenInfo>(cur_token),
                                                                                      temp_identifier));
            get_tok();
            lhs = rules[op].infix(std::move(lhs), op);
            op = cur_token;
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
            switch (cur_token) {
                case TOKEN_RCURLY:
                    return get_tok(), std::move(res);
                case TOKEN_EOF:
                    return parser_throws("expected } but found EOF");
                default:
                    res->lines.push_back(parse_statement());
                    if (res->lines.back() == nullptr) return nullptr;
            }
        }
    }


    //make sure that cur_token != TOKEN_EOF && cur_token != TOKEN_RCURLY on call
    unique_ptr<ast::Node> parse_statement() {
        auto res = parse_expression();
        if (!match(TOKEN_SEMICOLON)) return parser_throws("expected ; after expression but found " + token_to_string(
                    static_cast<TokenInfo>(cur_token), temp_identifier));
        return res;
    }

    ast::Program parse_program() {
        Program res;
        while (cur_token != TOKEN_EOF) {
            res.declarations.push_back(parse_function());
            if (panic_mode) panic();
        }
        return res;
    }

    unique_ptr<ast::Function> parse_function() {
        if (!match(TOKEN_FN)) return parser_throws("expected function");
        std::string cur_name;
        if (!match(TOKEN_IDENTIFIER)) {
            return parser_throws("expected function name");
        }
        cur_name = std::move(temp_identifier);
        temp_identifier = "";
        if (!match(TOKEN_LPAREN)) return parser_throws("expected (");
        if (!match(TOKEN_RPAREN)) return parser_throws("expected )");
        if (!match(TOKEN_LCURLY)) return parser_throws("expected {");
        FunctionSignature fsig{.name = cur_name};
        std::unique_ptr<Function> f = std::unique_ptr<Function>(new Function());
        f->signature = fsig;
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
        }
        throw std::runtime_error("token_to_string failed - internal error");
    }


    std::vector<std::string> get_errors() {
        return error_log;
    }
}
//TODO: add warning at 2 - -3 - are you sure?
//TODO: panic on error and output many errors