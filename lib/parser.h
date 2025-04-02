/*
здесь пишем только то, что должно быть видно извне и стараемся только
сигнатуры тут писать, пример ниже
*/
#ifndef CRYPT_PARSER_H
#define CRYPT_PARSER_H

#include <string>
#include <exception>
#include <memory>
#include "ast.h"

class AlwaysException : std::exception {
    std::string what;
public:

    AlwaysException(std::string s) {
        what = s;
    }
};

namespace parser {

    typedef std::unique_ptr<ast::Node> (*PrefixRule)();

    typedef std::unique_ptr<ast::Node> (*InfixRule)(std::unique_ptr<ast::Node> lhs, int op);

    enum Precedence {
        PREC_NONE,
        PREC_ASSIGN,
        PREC_ADD,
        PREC_FACTOR,
        PREC_UNARY,
        PREC_CALL,
        PREC_PRIMARY
    };
    struct RuleInfo {
        PrefixRule prefix;
        InfixRule infix;
        int precedence;
    };
    enum TokenInfo {
        TOKEN_EOF = 0,
        TOKEN_ADD,
        TOKEN_SUB,
        TOKEN_MUL,
        TOKEN_DIV,
        TOKEN_IDENTIFIER,
        TOKEN_INT_LIT,
        TOKEN_LPAREN,
        TOKEN_RPAREN,
        TOKEN_LCURLY,
        TOKEN_RCURLY,
        TOKEN_FN,
        TOKEN_ASSIGN,
        TOKEN_SEMICOLON,
        TOKEN_UNKNOWN,
    };
    constexpr int OPERATOR_EXPECTED = 1;
    constexpr int VALUE_EXPECTED = 2;
    constexpr int ANY_TOKEN_EXPECTED = OPERATOR_EXPECTED | VALUE_EXPECTED;

    std::string token_to_string(TokenInfo tok, std::string temp_data);

    std::vector<std::string> get_errors();

    void init_parser(std::istream &in);

    unique_ptr<ast::Node> parse_expression();

    unique_ptr<ast::Function> parse_function();

    unique_ptr<ast::Block> parse_block();

    unique_ptr<ast::Node> parse_statement();

    ast::Program parse_program();
}

/* Grammar */ /*`
 let's assume LL(1) grammar for now ( TODO: proof )
 (TODO: while?, arrays, map?...)
-   Types: int, (array?), (map?)

           function call
                \/
-    Operators: f(), -(unary), /, *, +, -, ==, !=, <=, >=, <, >, &&, ||, =(assignment)
  (priority in non-increasing order)

-  Function -> fn <name>(T1 <n1>, ...) ~> { Block }
-  Block -> (Statements | Expr)*
-  Statements ->
        if (Expr) { Block } |
        if (Expr) { Block } else { Block } |
        for (Expr; Expr; Expr) { Block } |
- Expr -> Var = Expr | Term
 Term -> кобинация тех операторов, Var, Number
 Var -> [a-zA-Z_][a-zA-Z0-9_]*


`*/

#endif