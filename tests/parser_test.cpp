#include "gtest/gtest.h"
#include "lib/parser.h"

using namespace testing;
using namespace parser;

// можно использовать параметры в тестах так:

using InvalidParserExceptionParamTestSuite = TestWithParam<std::string>;

TEST_P(InvalidParserExceptionParamTestSuite, Sample) {
    auto code = std::istringstream(GetParam());

    ASSERT_ANY_THROW(parse(code));
}

INSTANTIATE_TEST_SUITE_P(
    InvalidGroup,
    InvalidParserExceptionParamTestSuite,
    Values(
        "fn main() {", // missed }
        "fn main() {}$", // extra $
        "fn main() { int x = 5 }" // missed ;
        // etc
    )
);

using CorrectParserExceptionParamTestSuite = TestWithParam<std::string>;

TEST_P(CorrectParserExceptionParamTestSuite, CorrectSample) {
    auto code = std::istringstream(GetParam());
    ASSERT_NO_THROW(parse(code));
}

INSTANTIATE_TEST_SUITE_P(
    CorrectGroup,
    CorrectParserExceptionParamTestSuite,
    Values(
        "fn main() {}"
    )
);

using CorrectParserExpressionTestWithAnswer = Test;

std::string parse_res(std::string x) {
    std::stringstream ss(x);
    return parse(ss)->to_str1();
}
TEST(CorrectParserExpressionTestWithAnswer, ExampleTest) {
    ASSERT_EQ(parse_res("x"), "x");
    ASSERT_EQ(parse_res("y"), "y");
    ASSERT_EQ(parse_res("zf"), "zf");
    ASSERT_EQ(parse_res("12"), "12");
    ASSERT_EQ(parse_res("x * 2"), "(x*2)");
    ASSERT_EQ(parse_res("x * y + z - y"), "(((x*y)+z)-y)");
    ASSERT_EQ(parse_res("2 * 2 - 2 * 2"), "((2*2)-(2*2))");
}

