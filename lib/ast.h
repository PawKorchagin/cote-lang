//
// Created by motya on 28.03.2025.
//

#ifndef CRYPT_AST_H
#define CRYPT_AST_H
#include <string>
#include <utility>
#include <vector>
#include <stdexcept>
#include <memory>

template<typename T>
using unique_ptr = std::unique_ptr<T>;
namespace ast {
    enum class VarType {
        UNKNOWN,
        INT,
        //:)
    };
    enum class BinaryOpType {
        ADD = 0,
        SUB,
        DIV,
        MUL,
        ASSIGN,
        EQ,
        LE,
        LS,
        GR,
        GE,
        AND2,
        OR2,
        UNKNOWN
    };
    class Node {
    public:
        virtual ~Node() = default;
        virtual std::string to_str1() const;
    };
    class FunctionSignature {
        //TODO
    };
    class Block : public Node {
    public:
        std::vector<unique_ptr<Node>> lines;
        unique_ptr<Block> block;
    };
    class Function : public Node  {
    public:
        std::unique_ptr<FunctionSignature> signature;
        std::unique_ptr<Block> block;
    };
    class IntLitExpr : public Node {
    public:
        const int number;
        explicit IntLitExpr(int val):number(val) {  }
        [[nodiscard]] std::string to_str1() const override;
    };
    //actually any expression with identifier at first
    class VarExpr : public Node {
    public:
        VarType type;
        std::string name;
        explicit VarExpr(std::string  name): name(std::move(name)) {}
        std::string to_str1() const override;

    };
    template<BinaryOpType type>
    class BinaryExpr : public Node {
    public:
        unique_ptr<Node> l, r;
        BinaryExpr(unique_ptr<Node> l, unique_ptr<Node> r):l(std::move(l)), r(std::move(r)) {}

        static std::string operatorString() {
            switch (type) {
                case BinaryOpType::ADD: return "+";
                case BinaryOpType::SUB: return "-";
                case BinaryOpType::MUL: return "*";
                case BinaryOpType::DIV: return "/";
                default: throw std::invalid_argument("never see me");;
            }
        }
        std::string to_str1() const override {
            return "(" + l->to_str1() + operatorString() + r->to_str1() + ")";
        }
    };

    using AddExpr = BinaryExpr<BinaryOpType::ADD>;
    using DivExpr = BinaryExpr<BinaryOpType::DIV>;
    using MulExpr = BinaryExpr<BinaryOpType::MUL>;
    using SubExpr = BinaryExpr<BinaryOpType::SUB>;
}

#endif //CRYPT_AST_H
