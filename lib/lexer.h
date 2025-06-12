//
// Created by motya on 14.04.2025.
//

#ifndef CRYPT_LEXER_H
#define CRYPT_LEXER_H
#include <string>
namespace parser {
    enum TokenInfo {
        TOKEN_EOF = 0,
        TOKEN_ADD,
        TOKEN_SUB,
        TOKEN_MUL,
        TOKEN_DIV,
        TOKEN_MOD,
        TOKEN_IDENTIFIER,
        TOKEN_INT_LIT,
        TOKEN_STR_LIT,
        TOKEN_LBRACKET,
        TOKEN_RBRACKET,
        TOKEN_LPAREN,
        TOKEN_RPAREN,
        TOKEN_LCURLY,
        TOKEN_RCURLY,
        TOKEN_FN,
        TOKEN_ARROW,
        TOKEN_IF,
        TOKEN_ELSE,
        TOKEN_FOR,
        TOKEN_WHILE,
        TOKEN_RETURN,
        TOKEN_ASSIGN,
        TOKEN_PLUS_EQ,
        TOKEN_MINUS_EQ,
        TOKEN_MUL_EQ,
        TOKEN_DIV_EQ,
        TOKEN_AND,
        TOKEN_OR,
        TOKEN_EQ,
        TOKEN_NEQ,
        TOKEN_LS,
        TOKEN_LE,
        TOKEN_GR,
        TOKEN_GE,
        TOKEN_SEMICOLON,
        TOKEN_COMMA,
        TOKEN_DOT,
        TOKEN_UNKNOWN,
        TOKEN_COMMENT,//internal
    };

    struct TokenData {
        std::string identifier = "";
        int token = 0;
        int lines = 0;
        int cnt = 0;
    };
    extern TokenData cur;
    extern TokenData prv;


    void keep_newlines();
    void skip_newlines();

    void roll_back();

    int get_tok();
    void init_lexer(std::istream &in);


    std::string token_to_string(TokenInfo tok, std::string temp_data);
}

#endif //CRYPT_LEXER_H
