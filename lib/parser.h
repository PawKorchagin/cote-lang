/*
здесь пишем только то, что должно быть видно извне и стараемся только
сигнатуры тут писать, пример ниже
*/

#include <string>
#include <exception>
#include <memory>
#include "AST.h"

class AlwaysException : std::exception {
    std::string what;
public:

    AlwaysException(std::string s) {
        what = s;
    }
};

using namespace AST;
namespace Parser {
    typedef std::unique_ptr<Node> (*PrefixRule)();

    typedef std::unique_ptr<Node> (*InfixRule)(std::unique_ptr<Node> lhs, int op);

    enum Precedence {
        PREC_NONE,
        PREC_ASSIGN,
        PREC_ADD,
        PREC_FACTOR,
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
    };
    constexpr int OPERATOR_EXPECTED = 1;
    constexpr int VALUE_EXPECTED = 2;
    constexpr int ANY_TOKEN_EXPECTED = OPERATOR_EXPECTED | VALUE_EXPECTED;

    //guys you can test this
    std::unique_ptr<Node> parse(std::istream &in);

    //and this
    unique_ptr<Node> parse_expression();

    inline unique_ptr<Function> parseFunction() { throw AlwaysException("never");/* TODO */ }

    inline unique_ptr<Block> parseBlock() { throw AlwaysException("gonna");/* TODO */ }

    inline unique_ptr<Node> parseStatement() { throw AlwaysException("give you up");/* TODO */ }
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