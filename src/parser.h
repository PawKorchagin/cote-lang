/*
здесь пишем только то, что должно быть видно извне и стараемся только
сигнатуры тут писать, пример ниже
*/
#ifndef COTE_PARSER_H
#define COTE_PARSER_H

#include <string>
#include <exception>
#include <memory>
#include "ast.h"
#include "bytecode_emitter.h"

class AlwaysException : std::exception {
    std::string what;
public:

    AlwaysException(std::string s) {
        what = s;
    }
};

namespace parser {

    typedef std::unique_ptr<ast::Node> (*PrefixRule)();

    typedef std::unique_ptr<ast::Node> (*InfixRule)(std::unique_ptr<ast::Node> lhs);

    enum Precedence {
        PREC_NONE,
        PREC_TEMP,
        PREC_OR,// or
        PREC_AND,// and
        PREC_EQ,// == !=
        PREC_CMP,// <= < > >=
        PREC_ADD,// + -
        PREC_FACTOR,// * /
        PREC_UNARY,// - !(maybe)
        PREC_CALL,// [] () . TODO: ?., ?:
        PREC_PRIMARY
    };
    struct RuleInfo {
        PrefixRule prefix;
        InfixRule infix;
        int precedence;
        bool left_assoc = true;
    };
//TODO: forbid x = (y = z) expressions
    unique_ptr<ast::Node> parse_expression();

    void parse_function();

    void parse_return();

    void parse_block_();

    void parse_statement();

    void parse_annotations();

    ast::Program parse_program(interpreter::VMData& vm);

    void init_parser(std::istream& in, interpreter::BytecodeEmitter* emitter);

    bool epush(ast::Node* expr);
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