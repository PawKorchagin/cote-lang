#include "gtest/gtest.h"
#include "lib/parser.h"

using namespace testing;

// mожно использовать параметры в тестах так:

using ParserExceptionParamTestSuite = TestWithParam<std::string>;

TEST_P(ParserExceptionParamTestSuite, Sample) {
    auto code = GetParam();

    ASSERT_ANY_THROW(parse(code));
}

INSTANTIATE_TEST_SUITE_P(
    SampleGroup,
    ParserExceptionParamTestSuite,
    Values(
        "fn main() {", // missed }
        "fn main() {}$", // extra $
        "fn main() { int x = 5 }" // missed ;
        // etc
    )
);