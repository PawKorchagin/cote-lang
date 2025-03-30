//
// Created by motya on 28.03.2025.
//

#ifndef CRYPT_AST_H
#define CRYPT_AST_H
#include <string>
#include <vector>
#include <stdexcept>

template<typename T>
using unique_ptr = std::unique_ptr<T>;
namespace AST {
    class Node {
    public:
        virtual ~Node() = default;
        virtual std::string toStr1() { throw std::runtime_error("Should not be called"); }
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
        int number;
        IntLitExpr(int val):number(val) {  }
        std::string toStr1() override { return std::to_string(number); }
    };
    enum class VarType {
        UNKNOWN,
        INT,
        //:)
    };
    //actually any expression with identifier at first
    class VarExpr : public Node {
    public:
        VarType type;
        std::string name;
        VarExpr(const std::string& name): name(name) {}
        std::string toStr1() override { return name; }
    };
    enum class BinaryOpType : int {
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
    template<BinaryOpType type>
    class BinaryExpr : public Node {
    public:
        unique_ptr<Node> l, r;
        BinaryExpr(unique_ptr<Node> l, unique_ptr<Node> r):l(std::move(l)), r(std::move(r)) {}
        std::string_view get_string_op() { return "ff"; }
        std::string operatorString() {
            switch (type) {
                case BinaryOpType::ADD: return "+";
                case BinaryOpType::SUB: return "-";
                case BinaryOpType::MUL: return "*";
                case BinaryOpType::DIV: return "/";
            }
        }
        std::string toStr1() override { return "(" + l->toStr1() + operatorString() + r->toStr1() + ")"; }
    };

}

#endif //CRYPT_AST_H
