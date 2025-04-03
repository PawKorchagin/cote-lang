#include <iostream>
#include <sstream>

#include "lib/parser.h"
#include "lib/equals.h"

std::unique_ptr<ast::Node> parse(std::istream& in) {
    parser::init_parser(in);
    return parser::parse_expression();
}
int main() {
    using namespace ast;
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
    auto mulExpr = std::make_unique<BinaryExpr<BinaryOpType::MUL>>(std::move(x), std::move(three));

    // auto mulExpr2 = std::make_unique<BinaryExpr<BinaryOpType::MUL>>(std::move(addExpr), std::move(subExpr));
    // auto in = std::stringstream("(3 + x) * (7 - y)");
    auto in = std::stringstream("3 * x");
    auto expr = parse(in);
    
    try {
        std::cout << (ast::equals(expr.get(), mulExpr.get()));
    } catch (std::bad_cast& e) {
        std::cout << e.what() << "\n";
    } catch(std::exception& e) {
        std::cout << e.what() << "\n";
    }
    in = std::stringstream ("(x * 3)");
    std::cout << parse(in)->to_str1() << std::endl;

    // Print expression
    // std::cout << "Expression: " << mulExpr->to_str1() << std::endl;

    return 0;
}
