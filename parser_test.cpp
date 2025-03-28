#include "gtest/gtest.h"
#include "lib/parser.h"

using namespace testing;

//---Example---
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

// ---Utils for tests---

using MainScopeOKTest = TestWithParam<std::string>;
using MainScopeFailTest = TestWithParam<std::string>;

TEST_P(MainScopeOKTest, ValidScope) {
    std::string scope = GetParam();
    std::string code = "fn main() {" + scope + "}";

    EXPECT_NO_THROW(parse(code));
}

TEST_P(MainScopeFailTest, InvalidScope) {
    std::string scope = GetParam();
    std::string code = "fn main() {" + scope + "}";

    ASSERT_ANY_THROW(parse(code));
}

// ---Variables init tests---

INSTANTIATE_TEST_SUITE_P(
    initOKGroup,
    MainScopeOKTest,
    Values(
        "int a = 10;",   //simple initialization
        "int b = -100"   //negarive number           
        "int a = 0; int b = a;",        // init with another variable
        "int a = 1; int A = 2;", //register independent
        "int _ = 10;", //ignore name
        "int A_ = 0; int b1 = 2; int hello_world = 1; int world_HELLO = 228;", //different symbols in names
        "int a1 = 1; int a1234567890 = 2; int a1a2a3 = 3;" //numbers in names
        "int StR_A1nge00Na____00219321__Me = 10;", //everything in name
        "int a = 2147483647;", //max integer
        "int a = -2147483648;" //min integer
    )
);

INSTANTIATE_TEST_SUITE_P(
    InitFailGroup,
    MainScopeFailTest,
    Values(
        "int 1a = 1;", //name start with number
        "int a- = 1;", // - in name
        "int a++ = 1;",
        "a;", //no type no assign
        "a = 10;", //no type
        //"int a;" //no assign
        "int      = 10;", //whitespace name
        "int int = 10;", //name of a type
        "int for = 10;",
        "int if = 10;",
        "int >>> = 1;",
        "int return = 1", 
        "for = 0;",
        "int invalid name = 10;", //space in name
        "int a = 10000000000000;", //overflow 
        "int a = -100000000000000;",
        "int a@!ghf^@^#@ = 1;", //invalid symbols
        "int 😁😂 = 1;", //unicode symbols
        "int a = 1; int a = 1;" //initialize initialized
        "int a = b;" //init with unitialized variable
    )
);


