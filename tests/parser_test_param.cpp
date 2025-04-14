//
// Created by Георгий on 14.04.2025.
//

#include "utils.h"

using parser_exception_param_test_suite = TestWithParam<std::string>;
using parser_no_exception_param_test_suite = TestWithParam<std::string>;

TEST_P(parser_exception_param_test_suite, InvalidGroup) {
        const auto code = GetParam();
        ASSERT_ANY_THROW(parse(code));
}

// --gtest_filter=*

INSTANTIATE_TEST_SUITE_P(
        InvalidGroup,
        parser_exception_param_test_suite,
        Values(
                "fn main() {", // missed }
                "fn main() {}$", // extra $
                "fn main() { int x = 5 }" // missed ;
                // etc
        )
);

TEST_P(parser_exception_param_test_suite, CorrectGroup) {
        const auto code = GetParam();
        ASSERT_NO_THROW(parse(code));
}

INSTANTIATE_TEST_SUITE_P(
        CorrectGroup,
        parser_exception_param_test_suite,
        Values(
                "fn main() {}"
        )
);

using MainScopeOKTest = TestWithParam<std::string>;
using MainScopeFailTest = TestWithParam<std::string>;
/*
TEST_P(MainScopeOKTest, ValidScope) {
    std::string scope = GetParam();
    std::string code("fn main() {" + scope + "}");

    ASSERT_NO_THROW(parse(code));
}

TEST_P(MainScopeFailTest, InvalidScope) {
    const std::string scope = GetParam();
    std::string code = "fn main() {" + scope + "}";

    ASSERT_ANY_THROW(parse(code));
}

// ---Variables init tests---

INSTANTIATE_TEST_SUITE_P(
        initOKGroup,
        MainScopeOKTest,
        Values(
                "int a = 10;",   //simple initialization
                "int b = -100;"   //negarive number
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
                "int return = 1;",
                "for = 0;",
                "int invalid name = 10;", //space in name
                "int a = 10000000000000;", //overflow
                "int a = -100000000000000;",
                "int a = 2147483648;",
                "int a = -2147483649;"
                "int a@!ghf^@^#@ = 1;", //invalid symbols
                "int 😁😂 = 1;", //unicode symbols
                "int a = 1; int a = 1;" //initialize initialized
                "int a = b;" //init with unitialized variable
                "int  a  =  10  ;", //whitespaces
                "int\ta\t=\t10\t;"
        )
);

 */

int main(int argc, char** argv) {
        InitGoogleTest(&argc, argv);
        //std::cout << "Registered tests:\n";
        // auto& listeners = UnitTest::GetInstance()->listeners();
        // listeners.Append(new FileAndConsoleListener("logs/dump"));
        return RUN_ALL_TESTS();
}