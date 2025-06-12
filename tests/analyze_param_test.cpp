//
// Created by Георгий on 15.04.2025.
//

#include "utils.h"
#include "lib/semantics/analyzer.h"
#include "lib/ast.h"

inline auto parse_from_string(const std::string &text) {
    auto in = std::stringstream(text);
    parser::init_parser(in, new BytecodeEmitter());
    auto expr = parse_program();
    return expr;
}

using str_semantics_suite = TestWithParam<std::pair<std::string, std::string> >;

using namespace ast;

//inline auto run(const std::string &s) {
//    for (const auto &program = parse_from_string(s);
//         const auto &fn: program.declarations) {
//        auto tree = fn->clone_upcasting();
//        analysis::analyze(std::move(tree));
//    }
//}

//TEST_P(str_semantics_suite, lvalue_issues) {
//    auto &[incorrect, fixed] = GetParam();
//    EXPECT_THROW({
//                 for (const auto &program = parse_from_string(incorrect);
//                     const auto &fn: program.declarations) {
//                 // auto tree = fn->move_upcasting();
//                 analysis::analyze(fn->move_upcasting());
//                 }
//                 }, lvalue_error
//    );
//    EXPECT_NO_THROW({
//        for (const auto &program = parse_from_string(fixed);
//            const auto &fn: program.declarations) {
//        // auto tree = fn->move_upcasting();
//        analysis::analyze(fn->move_upcasting());
//        }
//        }
//    );
//}

// TEST_P(str_semantics_suite, rvalue_issues) {
//     EXPECT_THROW(
//         {
//         for (const auto &program = parse_from_string(GetParam());
//             const auto &fn: program.declarations) {
//         auto tree = fn->move_upcasting();
//         analysis::analyze(tree);
//         }
//         }, rvalue_error
//     );
// }

#define param(...) std::make_pair(__VA_ARGS__)

INSTANTIATE_TEST_SUITE_P(lvalue_issues,
                         str_semantics_suite,
                         Values(
                             param(
                                 "fn main() { 1 = 1; }",
                                 "fn main() { x = 1; }"
                             ),
                             param(
                                 "fn kek() { 1 = 1; }\nfn main() { kek(); }",
                                 "fn kek() { x = 1; }\nfn main() { kek(); }"
                             ),
                             param(
                                 "fn main() { kek(); }\nfn kek() { 1 = 1; }",
                                 "fn main() { kek(); }\nfn kek() { x = 1; }"
                             ),
                             param(
                                 "fn main() { kek() = 1; }\nfn kek() {}",
                                 "fn main() { x = kek(); }\nfn kek() { return 1; }"
                             )
                         )
);

/*
param(
"fn main() { l = array(10); l[0] = 1; l[2 * 3 + 1] = 2; l[2*l[1]] = l[2] = [3];  }"
"fn main() { l = array(10); l[0] = 1; l[2 * 3 + 1] = 2; l[l[1]] = l[2] = [3];  }"
)l
param(
"fn main() { fn(a){return 3;}[0] = 3; }"
"fn main() { return fn(a){ return 3; } }"
)
"fn main() { fn(a){return a[0];}()[1] = 2; }"
"fn main() { b[a[1][2*a+4]] = a[0]=3; }"
 */

// INSTANTIATE_TEST_SUITE_P(rvalue_issues,
//                          str_semantics_suite,
//                          Values(
//                              // "fn main() { x = x; }"
//                          ));
