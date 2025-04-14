#include "utils.h"

using parser_exception_param_test_suite = TestWithParam<std::string>;
using FunctionFromFileTestSuite = Test;

TEST(FunctionFromFileTestSuite, FileTests) {
    EXPECT_NO_THROW(parse_program_throws("../../tests/test1.ct"));
}

// ---Equals Tests---

using CorrectParserExpressionTestWithAnswer = Test;
using IncorrectParserExpressionTest = Test;
using RandomExpressionEqualsTest = Test;

std::string parse_res(std::string x) {
    return parse(std::move(x))->to_str1();
}

TEST(CorrectParserExpressionTestWithAnswer, ExampleTest) {
    ASSERT_EQ(parse_res("1 - - 2"), "(1--(2))");
    ASSERT_EQ(parse_res(" 3 "), "3");
    ASSERT_EQ(parse_res(" 3 "), "3");
    ASSERT_EQ(parse_res("-3"), "-3");
    ASSERT_EQ(parse_res(" - 3392032 "), "-(3392032)");
    EXPECT_EQ(parse_res("-9223372036854775808"), "-9223372036854775808");
    EXPECT_EQ(parse_res("9223372036854775807"), "9223372036854775807");
    EXPECT_EQ(parse_res("-9223372036854775807"), "-9223372036854775807");
    ASSERT_EQ(parse_res("_1"), "_1");
    ASSERT_EQ(parse_res("_2vdsda232"), "_2vdsda232");
    ASSERT_EQ(parse_res("___h_1_21___212"), "___h_1_21___212");
    ASSERT_EQ(parse_res("1 - - 2"), "(1--(2))");
    ASSERT_EQ(parse_res(" - - 1"), "-(-(1))");
    ASSERT_EQ(parse_res(" - -1"), "-(-1)");
    ASSERT_EQ(parse_res("- 2-2"), "(-(2)-2)");
    ASSERT_EQ(parse_res("- 2- 2"), "(-(2)-2)");
    ASSERT_EQ(parse_res("- 2 -2"), "(-(2)-2)");
    ASSERT_EQ(parse_res("-2 - -2"), "(-2--2)");
    ASSERT_EQ(parse_res("-2 - - 2"), "(-2--(2))");
    ASSERT_EQ(parse_res("(x)*y"), "(x*y)");
    ASSERT_EQ(parse_res("(x*y)*7"), "((x*y)*7)");
    ASSERT_EQ(parse_res("(((( (((x*7)*7))))))"), "((x*7)*7)");
    ASSERT_EQ(parse_res("x1"), "x1");
    ASSERT_EQ(parse_res("_1"), "_1");
    ASSERT_EQ(parse_res("1"), "1");
    ASSERT_EQ(parse_res("-1"), "-1");
    ASSERT_EQ(parse_res("y"), "y");
    ASSERT_EQ(parse_res("zf"), "zf");
    ASSERT_EQ(parse_res("12"), "12");
    ASSERT_EQ(parse_res("x * 2"), "(x*2)");
    ASSERT_EQ(parse_res("x * y + z - y"), "(((x*y)+z)-y)");
    ASSERT_EQ(parse_res(" 2 *2 - 2 * 2 "), "((2*2)-(2*2))");
    ASSERT_EQ(parse_res("x + y*  -x"), "(x+(y*-(x)))");
    ASSERT_EQ(parse_res("(3 + x) * (7 - y)"), "((3+x)*(7-y))"); // segmentation fault - fixed
}

TEST(IncorrectParserExpressionTest, ExampleTest) {
    EXPECT_THROW(parse_res("9223372036854775808"), std::runtime_error);
    EXPECT_THROW(parse_res("-9223372036854775809"), std::runtime_error);
    EXPECT_THROW(parse_res("-9223372036854775809"), std::runtime_error);
    EXPECT_THROW(parse_res("121b22"), std::runtime_error);
    EXPECT_THROW(parse_res("-2b"), std::runtime_error);
    EXPECT_THROW(parse_res("2b"), std::runtime_error);
    EXPECT_THROW(parse("(x*7"), std::runtime_error);
    EXPECT_THROW(parse("(x7"), std::runtime_error);
    //EXPECT_THROW(parse("7)"), std::runtime_error);
    //EXPECT_THROW(parse("1 + 2)"), std::runtime_error);
    EXPECT_THROW(parse("1x"), std::runtime_error);
    EXPECT_THROW(parse("()"), std::runtime_error);
    EXPECT_THROW(parse("*x"), std::runtime_error);
    EXPECT_THROW(parse("x*"), std::runtime_error);
    EXPECT_THROW(parse("x x"), std::runtime_error);
    //EXPECT_THROW(parse("y---1"), std::runtime_error);
    EXPECT_THROW(parse("--1"), std::runtime_error);
    EXPECT_THROW(parse("-- -1"), std::runtime_error);
    //EXPECT_THROW(parse("x--1"), std::runtime_error);
    EXPECT_THROW(parse("y++1"), std::runtime_error);
    EXPECT_THROW(parse("y+++1"), std::runtime_error);
    EXPECT_THROW(parse("++1"), std::runtime_error);
    EXPECT_THROW(parse("y++"), std::runtime_error);
    EXPECT_THROW(parse("x + y * 1 - "), std::runtime_error);
    EXPECT_THROW(parse("+"), std::runtime_error);
}

TEST(OverflowParserTest, ExampleTest) {
EXPECT_THROW(parse("100000000000"), std::runtime_error);
    EXPECT_THROW(parse("-1000000000000"), std::runtime_error);
    EXPECT_THROW(parse("2147483648"), std::runtime_error);
    EXPECT_THROW(parse("-2147483649"), std::runtime_error);

    EXPECT_NO_THROW(parse("2147483647"));
    EXPECT_NO_THROW(parse("-2147483648"));
}

enum ExpressionParts {
    
    ADD,
    SUB,
    MULT,
    DIV,
    //add here new operations if needed
    VAR,
    CONST,
    PARTS_COUNT
};

const std::string operations_tokens[] = {"+", "-", "*", "/"}; //add here operation as string
const std::string vars[3] = {"x", "y", "z"};
std::discrete_distribution<int> vars_distr {1,1,1};

std::mt19937 rnd; // better to use fix seed and mt19937

std::string rand_expr_rec(std::vector<int>& weights, std::default_random_engine& generator) {
    std::discrete_distribution<int> parts_distr(weights.begin(), weights.end());
    int part = parts_distr(generator);
    std::string expr;

    if (part < VAR) {
        weights[VAR]++;
        weights[CONST]++;
        std::string left = rand_expr_rec(weights, generator);
        std::string right = rand_expr_rec(weights, generator);
        expr = "(" + left + operations_tokens[part] + right + ")";
        weights[VAR]--;
        weights[CONST]--;
    }
    else if (part == VAR) {
        expr = vars[vars_distr(generator)];
    }
    else if (part == CONST) {
        expr = std::to_string(rnd() % 100);
    }
    else {
        throw std::runtime_error("Invalid expression part");
    }

    return expr;
}

std::string rand_expression() {
    rnd = std::mt19937(2); //better to use fix seed and param tests with seeds
    const int initial_weight = 10; //increase to get longer expressions
    std::vector<int> weights(PARTS_COUNT, initial_weight);
    std::random_device rd;
    std::default_random_engine generator(rd());

    return rand_expr_rec(weights, generator);
}

TEST(RandomExpressionEqualsTest, RandomTests) {
    const int test_amount = 1000;

    for (int i = 0; i < test_amount; ++i) {
        std::string exp = rand_expression();
        std::cerr << exp << std::endl;
        ASSERT_EQ(parse_res(exp), exp);
    }
}

#include <memory>
#include <sstream>

#include "lib/ast.h"
#include "lib/equals.h"

TEST(ManualEqualsTest, ParseExpressionTest) {
    auto three = std::make_unique<IntLitExpr>(3);
    auto x = std::make_unique<VarExpr>("x");
    auto manualMul = std::make_unique<MulExpr>(std::move(three), std::move(x));

    std::string ss("3 * x");
    auto parsedExpr = parse(ss);

    EXPECT_NO_THROW({
                        bool eq = equals(parsedExpr.get(), manualMul.get());
                        EXPECT_TRUE(eq);
                    });
}

// ???? call to implicitly-deleted copy constructor try to solve this TODO - fixed

std::unordered_map<int, Node*> mp_int {
        {1, new IntLitExpr(1)},
        {2, new IntLitExpr(2)},
        {3, new IntLitExpr(3)},
};

//TODO: steal from here: https://github.com/pannous/wasp/blob/main/source/tests.cpp