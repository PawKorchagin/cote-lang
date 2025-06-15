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
        do {
            cur.identifier.push_back(static_cast<char>(cur_char));
            cur_char = in.get();
        } while (isdigit(cur_char));
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
        if (cur.identifier == "and") return cur.token = TOKEN_AND;
        if (cur.identifier == "or") return cur.token = TOKEN_OR;
        if (cur.identifier == "fn") return cur.token = TOKEN_FN;
        if (cur.identifier == "break") return cur.token = TOKEN_BREAK;
        if (cur.identifier == "continue") return cur.token = TOKEN_BREAK;
        if (cur.identifier == "if") return cur.token = TOKEN_IF;
        if (cur.identifier == "for") return cur.token = TOKEN_FOR;
        if (cur.identifier == "while") return cur.token = TOKEN_WHILE;
        if (cur.identifier == "else") return cur.token = TOKEN_ELSE;
        if (cur.identifier == "return") return cur.token = TOKEN_RETURN;
        return cur.token = TOKEN_IDENTIFIER;
    }

    int tok_string_literal() {
        cur.identifier.clear();
        while ((cur_char = in.get()) != '"' && cur_char != EOF) {
            cur.identifier.push_back(static_cast<char>(cur_char));
        }
        if (cur_char != EOF) cur_char = in.get();
        return cur.token = TOKEN_STR_LIT;
    }


    int get_tok_core() {
        if (isdigit(cur_char))
            return tok_number();
        if (isalpha(cur_char) || cur_char == '_')
            return tok_identifier();
        if (cur_char == '"') return tok_string_literal();
        switch (cur_char) {
            case EOF:
                return cur.token = TOKEN_EOF;
            case '(':
                return helper_return_char(TOKEN_LPAREN);
            case ')':
                return helper_return_char(TOKEN_RPAREN);
            case '}':
                return helper_return_char(TOKEN_RCURLY);
            case '{':
                return helper_return_char(TOKEN_LCURLY);
            case ',':
                return helper_return_char(TOKEN_COMMA);
            case '+':
                return parse_token2('=', TOKEN_ADD, TOKEN_PLUS_EQ);
            case '.':
                return helper_return_char(TOKEN_DOT);
            case '%':
                return helper_return_char(TOKEN_MOD);
            case '-':
                cur_char = in.get();
                if (cur_char == '-')
                    parser_throws("illegal token --");
                if (cur_char == '=') return helper_return_char(TOKEN_MINUS_EQ);
                return cur.token = TOKEN_SUB;
            case '*':
                return parse_token2('=', TOKEN_MUL, TOKEN_MUL_EQ);
            case '/':
                cur_char = in.get();
                if (cur_char == '/') {
                    while (cur_char != '\n') cur_char = in.get();
                    return cur.token = TOKEN_COMMENT;
                }
                if (cur_char == '=') return helper_return_char(TOKEN_DIV_EQ);
                return cur.token = TOKEN_DIV;
            case '[':
                return helper_return_char(TOKEN_LBRACKET);
            case ']':
                return helper_return_char(TOKEN_RBRACKET);
            case '!':
                return parse_token2('=', TOKEN_UNKNOWN, TOKEN_NEQ);
            case '=':
                return parse_token2('=', TOKEN_ASSIGN, TOKEN_EQ);
            case ';':
                return helper_return_char(TOKEN_SEMICOLON);
            case '<':
                return parse_token2('=', TOKEN_LS, TOKEN_LE);
            case '>':
                return parse_token2('=', TOKEN_GR, TOKEN_GE);
            default:
                parser_throws("unknown token " + std::string(1, (char) cur_char));
                helper_return_char(TOKEN_UNKNOWN);
                return TOKEN_UNKNOWN;

        }
    }

    void roll_back() {
        nxt = std::move(cur);
        cur = std::move(prv);
        prv.token = TOKEN_UNKNOWN;
    }

    int get_tok() {
        if (nxt.token != -1) {
            cur = std::move(nxt);
            prv = std::move(cur);
            nxt.token = -1;
            return cur.token;
        }
        prv = std::move(cur);
        do {
            while (isspace(cur_char)) cur_char = in.get();
            cur.lines = in.plines();
            cur.cnt = in.pcnt();
            get_tok_core();
            if (cur.token == TOKEN_UNKNOWN) {
                parser_throws("unknown token " + std::string(1, (char) cur_char)), helper_return_char(
                        TOKEN_UNKNOWN);
            }
        } while (cur.token == TOKEN_COMMENT);
        return cur.token;
    }

    void init_lexer(std::istream &input_stream) {
        in.~SimpleStream();
        new(m_stream_ptr) SimpleStream(input_stream);
        cur_char = ' ';
        get_tok();
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
            case TOKEN_RETURN:
                return "return";
            case TOKEN_LBRACKET:
                return "]";
            case TOKEN_RBRACKET:
                return "[";
            case TOKEN_DOT:
                return ".";
            case TOKEN_MOD:
                return "%";
            case TOKEN_FOR:
                return "for";
            case TOKEN_AND:
                return "and";
            case TOKEN_OR:
                return "or";
            case TOKEN_STR_LIT:
                return '"' + temp_data + '"';
            case TOKEN_CONTINUE:
                break;
            case TOKEN_BREAK:
                break;
            case TOKEN_PLUS_EQ:
                break;
            case TOKEN_MINUS_EQ:
                break;
            case TOKEN_MUL_EQ:
                break;
            case TOKEN_DIV_EQ:
                break;
            case TOKEN_NEQ:
                break;
            case TOKEN_COMMENT:
                break;
        }
        throw std::runtime_error("token_to_string failed - internal error");
    }
}