//
// Created by Георгий on 15.04.2025.
//

#include "utils.h"
#include "lib/semantics/analyzer.h"
#include "lib/ast.h"

inline auto parse_from_string(const std::string &text) {
    auto in = std::stringstream(text);
    parser::init_parser(in);
    auto expr = parse_program();
    return expr;
}

using str_semantics_suite = TestWithParam<std::string>;

using namespace ast;

inline auto run(const std::string &s) {
    for (const auto &program = parse_from_string(s);
         const auto &fn: program.declarations) {
        auto tree = fn->clone_upcasting();
        analysis::analyze(tree);
    }
}

TEST_P(str_semantics_suite, lvalue_issues) {
    EXPECT_THROW(
        {
        for (const auto &program = parse_from_string(GetParam());
            const auto &fn: program.declarations) {
        auto tree = fn->clone_upcasting();
        analysis::analyze(tree);
        }
        }, lvalue_error
    );
}

TEST_P(str_semantics_suite, rvalue_issues) {
    EXPECT_THROW(
        {
        for (const auto &program = parse_from_string(GetParam());
            const auto &fn: program.declarations) {
        auto tree = fn->clone_upcasting();
        analysis::analyze(tree);
        }
        }, rvalue_error
    );
}

INSTANTIATE_TEST_SUITE_P(lvalue_issues,
                         str_semantics_suite,
                         Values(
                             "fn main() { 1 = 1; }",
                             "fn kek() { 1 = 1; }\nfn main() { kek(); }",
                             "fn main() { kek(); }\nfn kek() { 1 = 1; }",
                             "fn main() { kek() = 1; }\nfn kek() {}"
                         ));

INSTANTIATE_TEST_SUITE_P(rvalue_issues,
                         str_semantics_suite,
                         Values(
                             // "fn main() { x = x; }"
                         ));
