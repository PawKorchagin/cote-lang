/*
здесь пишем только то, что должно быть видно извне и стараемся только
сигнатуры тут писать, пример ниже
*/

#include <string>
#include <exception>
#include <memory>
#include "Expr.h"

class AlwaysException;


std::unique_ptr<Expr> parse(std::istream &in);

/* Grammar */ /*`
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