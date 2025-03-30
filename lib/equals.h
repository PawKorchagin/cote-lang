#ifndef EQUALS_H
#define EQUALS_H

#include <typeinfo>
#include "ast.h"
// #include "debug.h"

namespace AST {
    bool equals(const Node*, const Node*);
    //
    // inline bool operator==(const Node& lhs, const Node& rhs) {
    //     return equals(&lhs, &rhs);
    // }
    //
    // inline bool operator!=(const Node& lhs, const Node& rhs) {
    //     return !equals(&lhs, &rhs);
    // }
    //
    // inline bool operator==(const Node* lhs, const Node* rhs) {
    //     return equals(lhs, rhs);
    // }
    //
    // inline bool operator!=(const Node* lhs, const Node* rhs) {
    //     return !equals(lhs, rhs);
    // }

    template<typename Type>
    const Type* downcast(const Node* node) {
        static_assert(std::is_base_of_v<Node, Type>, "downcast: Type must be derived from Node");
        return dynamic_cast<const Type*>(node);
    }

    inline bool equals(const Node* lhs, const Node* rhs) {
        if (lhs == nullptr && rhs == nullptr) return true;
        // debug(lhs);
        // debug(rhs);
        if ((lhs == nullptr) || (rhs == nullptr)) return false;

        if (const auto *left_const = downcast<IntLitExpr>(lhs),
                     *right_const = downcast<IntLitExpr>(rhs); left_const && right_const) {
            // debug(left_const->to_str1());
            // debug(right_const->to_str1());
            return left_const->number == right_const->number;
        }

        if (const auto *left_var = downcast<VarExpr>(lhs),
                     *right_var = downcast<VarExpr>(rhs); left_var && right_var) {
            // debug(left_var->to_str1());
            // debug(right_var->to_str1());
            return left_var->name == right_var->name && left_var->type == right_var->type;
        }

        if (const auto *left_bin = downcast<AddExpr>(lhs),
                     *right_bin = downcast<AddExpr>(rhs); left_bin && right_bin) {
            return equals(left_bin->l.get(), right_bin->l.get()) &&
                   equals(left_bin->r.get(), right_bin->r.get());
        }

        if (const auto *left_bin = downcast<SubExpr>(lhs),
                     *right_bin = downcast<SubExpr>(rhs); left_bin && right_bin) {
            return equals(left_bin->l.get(), right_bin->l.get()) &&
                   equals(left_bin->r.get(), right_bin->r.get());
        }

        if (const auto *left_bin = downcast<MulExpr>(lhs),
                     *right_bin = downcast<MulExpr>(rhs); left_bin && right_bin) {
            // debug(left_bin->to_str1());
            // debug(right_bin->to_str1());
            return equals(left_bin->l.get(), right_bin->l.get()) &&
                   equals(left_bin->r.get(), right_bin->r.get());
        }

        if (const auto *left_bin = downcast<DivExpr>(lhs),
                     *right_bin = downcast<DivExpr>(rhs); left_bin && right_bin) {
            return equals(left_bin->l.get(), right_bin->l.get()) &&
                   equals(left_bin->r.get(), right_bin->r.get());
        }

        throw std::bad_cast();
    }
} // namespace AST

#endif // EQUALS_H
