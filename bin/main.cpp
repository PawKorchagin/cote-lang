#include <iostream>
#include "lib/parser.h"
#include "lib/equals.h"

using namespace AST;

int main() {
    std::cout << "crypt\n";
    auto three = std::make_unique<IntLitExpr>(3);
    auto seven = std::make_unique<IntLitExpr>(7);
    auto x = std::make_unique<VarExpr>("x");
    auto y = std::make_unique<VarExpr>("y");

    // Create (3 + x)
    auto addExpr = std::make_unique<BinaryExpr<BinaryOpType::ADD>>(std::move(three), std::move(x));

    // Create (7 - y)
    auto subExpr = std::make_unique<BinaryExpr<BinaryOpType::SUB>>(std::move(seven), std::move(y));

    // Create (3 + x) * (7 - y)
    auto mulExpr = std::make_unique<BinaryExpr<BinaryOpType::MUL>>(std::move(addExpr), std::move(subExpr));
    auto mulExpr2 = std::make_unique<BinaryExpr<BinaryOpType::MUL>>(std::move(addExpr), std::move(subExpr));

    std::cout << (equals(mulExpr.get(), mulExpr2.get()));

    // Print expression
    std::cout << "Expression: " << mulExpr->toStr1() << std::endl;

    return 0;
}
