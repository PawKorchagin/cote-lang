#include "gtest/gtest.h"
#include "lib/parser.h"

using namespace testing;

// можно использовать параметры в тестах так:

using InvalidParserExceptionParamTestSuite = TestWithParam<std::string>;

TEST_P(InvalidParserExceptionParamTestSuite, Sample) {
    auto code = GetParam();

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
    auto code = GetParam();
    ASSERT_NO_THROW(parse(code));
}

INSTANTIATE_TEST_SUITE_P(
    CorrectGroup,
    CorrectParserExceptionParamTestSuite,
    Values(
        "fn main() {}"
    )
);