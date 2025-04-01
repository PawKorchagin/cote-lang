#ifndef EQUALS_H
#define EQUALS_H

#include "ast.h"
#include "concept.h"
// #include "debug.h"

namespace ast {
    bool equals(const ast::Node*, const ast::Node*);

    inline bool Node::operator==(const Node& other) const {
        return equals(this, &other);
    }

    inline bool Node::operator==(const Node* other) const {
        return equals(this, other);
    }

    inline bool Node::operator!=(const Node* other) const {
        return !equals(this, other);
    }

    inline bool operator==(const Node& lhs, const Node* rhs) {
        return lhs.operator==(rhs);
    }

    inline bool operator!=(const Node& lhs, const Node* rhs) {
        return lhs.operator!=(rhs);
    }

    inline bool operator!=(const Node& lhs, const Node& rhs) {
        return lhs.operator!=(&rhs);
    }

    template<typename Type>
    const Type* downcast(const Node* node) {
        static_assert(std::is_base_of_v<Node, Type>, "downcast: Type must be derived from Node");
        return dynamic_cast<const Type*>(node);
    }

    // pay O(n^2) but have commutative checking
    template<typename CommutativeOperation> requires is_commutative<CommutativeOperation>
    bool commutative_step(CommutativeOperation left_bin, CommutativeOperation right_bin) {
        return step(left_bin, right_bin) ||
            equals(left_bin->r.get(), right_bin->l.get()) &&
                equals(left_bin->l.get(), right_bin->r.get());
    }

    template<typename BinaryOperation> requires is_binary_operation<BinaryOperation>
    bool step(BinaryOperation left_bin, BinaryOperation right_bin) {
        return equals(left_bin->l.get(), right_bin->l.get()) &&
            equals(left_bin->r.get(), right_bin->r.get());
    }

    inline bool equals(const Node* lhs, const Node* rhs) {
        if (lhs == nullptr && rhs == nullptr) return true;
        if ((lhs == nullptr) || (rhs == nullptr)) return false;

        if (const auto *left_const = downcast<IntLitExpr>(lhs),
                     *right_const = downcast<IntLitExpr>(rhs); left_const && right_const) {
            return left_const->number == right_const->number;
        }

        if (const auto *left_var = downcast<VarExpr>(lhs),
                     *right_var = downcast<VarExpr>(rhs); left_var && right_var) {
            return left_var->name == right_var->name && left_var->type == right_var->type;
        }

        if (const auto *left_bin = downcast<AddExpr>(lhs),
                     *right_bin = downcast<AddExpr>(rhs); left_bin && right_bin) {
            return commutative_step(left_bin, right_bin);
        }

        if (const auto *left_bin = downcast<SubExpr>(lhs),
                     *right_bin = downcast<SubExpr>(rhs); left_bin && right_bin) {
            return step(left_bin, right_bin);
        }

        if (const auto *left_bin = downcast<MulExpr>(lhs),
                     *right_bin = downcast<MulExpr>(rhs); left_bin && right_bin) {
            return commutative_step(left_bin, right_bin);
        }

        if (const auto *left_bin = downcast<DivExpr>(lhs),
                     *right_bin = downcast<DivExpr>(rhs); left_bin && right_bin) {
            return step(left_bin, right_bin);
        }

        return false;
    }
} // namespace AST

#endif // EQUALS_H
