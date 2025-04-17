//
// Created by motya on 14.04.2025.
//
#include "parser.h"
#include "exceptions.h"
#include "misc.h"
#include "lexer.h"

namespace parser {
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

    int cur_char = ' ';
    SimpleStream in(NULL_STREAM);
    void *m_stream_ptr = &in;

    TokenData cur = {};
    TokenData prv = {};
    TokenData nxt = {.token = -1};

    int helper_return_char(int TOKEN) {
        cur_char = in.get();
        return cur.token = TOKEN;
    }

    int tok_number() {
        cur.identifier.clear();
        cur.identifier.push_back(static_cast<char>(cur_char));
        if (cur_char == '-') {
            cur_char = in.get();
            if (cur_char == '-') return parser_throws("illegal token --"), cur.token = TOKEN_UNKNOWN;
            if (cur_char == '>') return helper_return_char(TOKEN_ARROW);
            if (!isdigit(cur_char)) return cur.token = TOKEN_SUB;
        }
        else cur_char = in.get();
        while (isdigit(cur_char)) {
            cur.identifier.push_back(static_cast<char>(cur_char));
            cur_char = in.get();
        }
        return cur.token = TOKEN_INT_LIT;
    }

    template<bool readFirst = true>
    int parse_token2(char second, int res1, int res2) {
        if constexpr (readFirst) {
            cur_char = in.get();
        }
        if (cur_char == second) return helper_return_char(res2);
        return cur.token = res1;
    }

    int tok_identifier() {
        cur.identifier.clear();
        do {
            cur.identifier.push_back(static_cast<char>(cur_char));
            cur_char = in.get();
        } while (isalnum(cur_char) || cur_char == '_');
        if (cur.identifier == "and") return cur.token = TOKEN_AND;
        if (cur.identifier == "or") return cur.token = TOKEN_OR;
        if (cur.identifier == "fn") return cur.token = TOKEN_FN;
        if (cur.identifier == "if") return cur.token = TOKEN_IF;
        if (cur.identifier == "for") return cur.token = TOKEN_FOR;
        if (cur.identifier == "while") return cur.token = TOKEN_WHILE;
        if (cur.identifier == "else") return cur.token = TOKEN_ELSE;
        if (cur.identifier == "return") return cur.token = TOKEN_RETURN;
        return cur.token = TOKEN_IDENTIFIER;
    }

    int tok_string_literal() {
        cur.identifier.clear();
        while((cur_char = in.get()) !=  '"' && cur_char != EOF) {
            cur.identifier.push_back(static_cast<char>(cur_char));
        }
        if (cur_char != EOF) cur_char = in.get();
        return cur.token = TOKEN_STR_LIT;
    }


    int get_tok_core(int token_info) {
        while (isspace(cur_char)) cur_char = in.get();
        if (isdigit(cur_char) || (token_info & VALUE_EXPECTED) && cur_char == '-')
            return tok_number();
        if (isalpha(cur_char) || cur_char == '_')
            return tok_identifier();
        if (cur_char == '"') return tok_string_literal();
        switch (cur_char) {
            case EOF: return cur.token = TOKEN_EOF;
            case '(': return helper_return_char(TOKEN_LPAREN);
            case ')': return helper_return_char(TOKEN_RPAREN);
            case '}': return helper_return_char(TOKEN_RCURLY);
            case '{': return helper_return_char(TOKEN_LCURLY);
            case ',':
                return helper_return_char(TOKEN_COMMA);
            case '+':
                return helper_return_char(TOKEN_ADD);
            case '.':
                return helper_return_char(TOKEN_DOT);
            case '%':
                return helper_return_char(TOKEN_MOD);
            case '-':
                cur_char = in.get();
                if (cur_char == '-')
                    return parser_throws("illegal token --"), cur.token = TOKEN_UNKNOWN;
                return parse_token2<false>('>', TOKEN_SUB, TOKEN_ARROW);
            case '*':
                return helper_return_char(TOKEN_MUL);
            case '/':
                cur_char = in.get();
                if (cur_char == '/') {
                    while (cur_char != '\n') cur_char = in.get();
                    return get_tok(token_info);
                }
                return cur.token = TOKEN_DIV;
            case '[':
                return helper_return_char(TOKEN_LBRACKET);
            case ']':
                return helper_return_char(TOKEN_RBRACKET);
            case '=':
                return parse_token2('=', TOKEN_ASSIGN, TOKEN_EQ);
            case ';':
                return helper_return_char(TOKEN_SEMICOLON);
            case '<':
                return parse_token2('=', TOKEN_LS, TOKEN_LE);
            case '>':
                return parse_token2('=', TOKEN_GR, TOKEN_GE);
            default:
                return parser_throws("unknown token " + std::string(1, (char)cur_char)), helper_return_char(
                        TOKEN_UNKNOWN);

        }
    }

    void roll_back() {
        nxt = std::move(cur);
        cur = std::move(prv);
        prv.token = TOKEN_UNKNOWN;
    }

    int get_tok(int token_info) {
        if (nxt.token != -1) {
            cur = std::move(nxt);
            prv = std::move(cur);
            nxt.token = -1;
            return cur.token;
        }
        prv = std::move(cur);
        cur.lines = in.plines();
        cur.cnt = in.pcnt();
        return get_tok_core(token_info);
    }

    void init_lexer(std::istream &input_stream) {
        in.~SimpleStream();
        new(m_stream_ptr) SimpleStream(input_stream);
        cur_char = ' ';
        get_tok();
    }
}