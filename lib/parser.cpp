#include "parser.h"
#include <fstream>
#include <map>
#include <cassert>
#include <array>


namespace {
    using namespace ast;
    using namespace parser;
    std::vector<int> line_breaks;

    //wrapper for keeping cursor position and error messages
    struct SimpleStream {
        SimpleStream(std::istream &in) : in(in) {}

        int get() {
            int c = in.get();
            if (c == '\n') {
                line_breaks.push_back(in.tellg());
            }
            return c;
        }

    private:
        std::istream &in;
    };

    std::string temp_identifier;
    std::vector<std::string> error_log;
    int cur_char = ' ';
    int cur_token = 0;
    std::istream NULL_STREAM(nullptr);
    SimpleStream in(NULL_STREAM);
    void *m_stream_ptr = &in;

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
        if (temp_identifier.size() < expected_size) return TOKEN_SUB;
        return cur_token = TOKEN_INT_LIT;
    }

    int tok_identifier() {
        temp_identifier.clear();
        do {
            temp_identifier.push_back(static_cast<char>(cur_char));
            cur_char = in.get();
        } while (isalnum(cur_char));
        return cur_token = TOKEN_IDENTIFIER;
    }

    int get_tok(int token_info = ANY_TOKEN_EXPECTED) {
        while (cur_char == ' ') cur_char = in.get();
        if (isdigit(cur_char) || (token_info & VALUE_EXPECTED) && cur_char == '-')
            return tok_number();
        if (isalpha(cur_char) || cur_char == '_')
            return tok_identifier();
        if (cur_char == '(') return helper_return_char(TOKEN_LPAREN);
        if (cur_char == ')') return helper_return_char(TOKEN_RPAREN);
        if (cur_char == -1) return cur_token = TOKEN_EOF;
        switch (cur_char) {
            case '+':
                return helper_return_char(TOKEN_ADD);
            case '-':
                return helper_return_char(TOKEN_SUB);
            case '*':
                return helper_return_char(TOKEN_MUL);
            case '/':
                return helper_return_char(TOKEN_DIV);
            default:
                return TOKEN_UNKNOWN;
        }
    }

//-----------------LEXER----------------------
// ---------------ERROR SYSTEM-----------------

    bool panic_mode = false;

//stops at ; or declaration or statement
    void panic() {
        while (true) {
            switch (cur_token) {
                case TOKEN_EOF:
                    return;
                default:
                    get_tok();
            }
        }
        //TODO
    }

    std::unique_ptr<Node> parser_throws(const std::string &message) {
        if (panic_mode) return nullptr;
        panic_mode = true;

        error_log.push_back(message);
        panic();
        return nullptr;
    }

// ---------------ERROR SYSTEM-----------------

    bool match(int token_type) {
        if (token_type != cur_token) return false;
        get_tok();
        return true;
    }

    unique_ptr<Node> parse_precedence(int prec);

    std::unique_ptr<Node> pfn_identifier() { return std::make_unique<VarExpr>(temp_identifier); }

    std::unique_ptr<Node> pfn_number() { return std::make_unique<IntLitExpr>(atoi(temp_identifier.c_str())); }

    std::unique_ptr<Node> pfn_grouping() {
        if (!match(TOKEN_LPAREN)) return parser_throws("expected ( but found");
        auto expr = parse_expression();
        if (cur_token != TOKEN_RPAREN) return parser_throws("expected ) but found");
        return expr;
    }

    std::unique_ptr<Node> ifn_binary(std::unique_ptr<Node> lhs, int op);

// clang-format off
// @formatter:off

    RuleInfo rules[] = {
    /* TOKEN_EOF */         {nullptr,        nullptr,    PREC_NONE},
    /* TOKEN_ADD */         {nullptr,        ifn_binary, PREC_ADD},
    /* TOKEN_SUB */         {nullptr,        ifn_binary, PREC_ADD},
    /* TOKEN_MUL */         {nullptr,        ifn_binary, PREC_FACTOR},
    /* TOKEN_DIV */         {nullptr,        ifn_binary, PREC_FACTOR},
    /* TOKEN_IDENTIFIER */  {pfn_identifier, nullptr,    PREC_PRIMARY},
    /* TOKEN_INT_LIT */     {pfn_number,     nullptr,    PREC_PRIMARY},
    /* TOKEN_LPAREN */      {pfn_grouping,   nullptr,    PREC_CALL},
    /* TOKEN_RPAREN */      {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_UNKNOWN */     {nullptr,   nullptr,    PREC_NONE},
    };
// clang-format on
// @formatter:on

    std::unique_ptr<Node> ifn_binary(std::unique_ptr<Node> lhs, int op) {
        auto rhs = parse_precedence(rules[op].precedence +
                                    1);//because we want all operators to be left-assoc(if right-assoc needed just do not add 1)
        if (rhs == nullptr) return parser_throws("expected value but found ");
        switch (op) {
            case TOKEN_ADD:
                return std::make_unique<AddExpr>(std::move(lhs), std::move(rhs));
            case TOKEN_SUB:
                return std::make_unique<SubExpr>(std::move(lhs), std::move(rhs));
            case TOKEN_MUL:
                return std::make_unique<MulExpr>(std::move(lhs), std::move(rhs));
            case TOKEN_DIV:
                return std::make_unique<DivExpr>(std::move(lhs), std::move(rhs));
            default:
                return parser_throws("Unknown binary operator | or internal error");
        }
    }

    unique_ptr<Node> parse_precedence(int prec) {
        if (rules[cur_token].prefix == nullptr)
            return parser_throws("prefix shouldn't be null");
        auto lhs = rules[cur_token].prefix();
        int op = get_tok(OPERATOR_EXPECTED);
        while (prec <= rules[op].precedence) {
            get_tok();
            if (rules[op].infix == nullptr) return parser_throws("expected operator but found ");
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

    std::vector<std::string> get_errors() {
        return error_log;
    }
}

//TODO: panic on error and output many errors