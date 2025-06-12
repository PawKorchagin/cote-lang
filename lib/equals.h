#ifndef EQUALS_H
#define EQUALS_H

#include "ast.h"
#include "concept.h"
// #include "debug.h"

namespace ast {
    bool equals(const ast::Node *, const ast::Node *);

    inline bool Node::operator==(const Node &other) const {
        return equals(this, &other);
    }

    inline bool Node::operator==(const Node *other) const {
        return equals(this, other);
    }

    inline bool Node::operator!=(const Node *other) const {
        return !equals(this, other);
    }

    inline bool operator==(const Node &lhs, const Node *rhs) {
        return lhs.operator==(rhs);
    }

    inline bool operator!=(const Node &lhs, const Node *rhs) {
        return lhs.operator!=(rhs);
    }

    inline bool operator!=(const Node &lhs, const Node &rhs) {
        return lhs.operator!=(&rhs);
    }

    template<typename Type>
    const Type *downcast(const Node *node) {
        static_assert(std::is_base_of_v<Node, Type>, "downcast: Type must be derived from Node");
        return dynamic_cast<const Type *>(node);
    }

    // pay O(n^2) but have commutative checking
    // template<typename CommutativeOperation>
    // // requires is_commutative<CommutativeOperation>
    // bool commutative_step(CommutativeOperation left_bin, CommutativeOperation right_bin) {
    //     return step(left_bin, right_bin) ||
    //            equals(left_bin->r.get(), right_bin->l.get()) &&
    //            equals(left_bin->l.get(), right_bin->r.get());
    // }

    // template<typename BinaryOperation>
    // requires is_binary_operation<BinaryOperation>
    // bool step(BinaryOperation left_bin, BinaryOperation right_bin) {
    //     return equals(left_bin->l.get(), right_bin->l.get()) &&
    //            equals(left_bin->r.get(), right_bin->r.get());
    // }
#define EQ_BINARY_HELPER_F(T) { auto l = static_cast<const BinaryExpr<T>*>(lhs); \
    auto r = static_cast<const BinaryExpr<T>*>(rhs);                             \
    return equals(l->l.get(), r->l.get()) && equals(l->r.get(), r->r.get()); }

    inline bool equals(const Node *lhs, const Node *rhs) {
        if (lhs->get_type() != rhs->get_type()) return false;
        switch (lhs->get_type()) {
            case NodeType::UnaryMinus:
                return equals(lhs, rhs);
            case NodeType::Block:
                throw std::runtime_error("why?");
            case NodeType::FunctionDef:
                throw std::runtime_error("why?");
            case NodeType::FunctionCall:
                throw std::runtime_error("why?");
            case NodeType::ArrayGet: {
                auto l = static_cast<const ArrayGet *>(lhs);
                auto r = static_cast<const ArrayGet *>(rhs);
                return equals(l->index.get(), r->index.get()) && equals(l->name_expr.get(), r->name_expr.get());
            }
            case NodeType::Return:
                throw std::runtime_error("why?");
            case NodeType::IntLit: {
                auto l = static_cast<const IntLitExpr *>(lhs);
                auto r = static_cast<const IntLitExpr *>(rhs);
                return l->number == r->number;
            }
            case NodeType::Var: {
                auto l = static_cast<const VarExpr *>(lhs);
                auto r = static_cast<const VarExpr *>(rhs);
                return l->name == r->name;
            }
            case NodeType::If:
                throw std::runtime_error("why? in equals unimplemented if");
            case NodeType::While:
                throw std::runtime_error("why? bc while unimpl in equals");
            case NodeType::BinaryPlus: EQ_BINARY_HELPER_F(BinaryOpType::ADD);
            case NodeType::BinaryMul: EQ_BINARY_HELPER_F(BinaryOpType::MUL);
            case NodeType::BinaryMinus: EQ_BINARY_HELPER_F(BinaryOpType::SUB);
            case NodeType::BinaryDiv: EQ_BINARY_HELPER_F(BinaryOpType::DIV);
        }
        return false;
    }

#undef EQ_BINARY_HELPER_F

} // namespace AST

#endif // EQUALS_H
