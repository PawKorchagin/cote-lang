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
    enum class UnaryOpType {
        MINUS,
    };

    enum class NodeType {
        Block,
        FunctionDef,
        FunctionCall,
        ArrayGet,
        Return,
        IntLit,
        Var,
        If,
        While,
        UnaryMinus,
        BinaryPlus,
        BinaryMul,
        BinaryDiv,
        BinaryMinus,
    };
    class Node {
    public:
        virtual ~Node() = default;

        virtual std::string to_str1() const;

        virtual NodeType get_type() const = 0;

        bool operator==(const Node &other) const;

        bool operator==(const Node *) const;

        bool operator!=(const Node *) const;
    };

    class FunctionSignature {
    public:
        std::string name;
        std::vector<std::string> params;
        //TODO
    };

    class Block : public Node {
    public:
        std::vector<unique_ptr<Node>> lines;
        inline NodeType get_type() const override { return NodeType::Block; }
    };

    class FunctionDef : public Node {
    public:
        FunctionSignature signature;
        std::unique_ptr<Block> block;
        inline NodeType get_type() const override { return NodeType::FunctionDef; }
    };
    class FunctionCall : public Node {
    public:
        std::unique_ptr<Node> name_expr;
        std::vector<std::unique_ptr<Node>> args;
        explicit FunctionCall(std::unique_ptr<Node> name_expr):name_expr(std::move(name_expr)) {}
        inline NodeType get_type() const override { return NodeType::FunctionCall; }
    };
    class ArrayGet : public Node {
    public:
        std::unique_ptr<Node> name_expr;
        std::unique_ptr<Node> index;
        ArrayGet(unique_ptr<Node> name_expr, std::unique_ptr<Node> index) : name_expr(std::move(name_expr)), index(std::move(index)) {}
        inline NodeType get_type() const override { return NodeType::ArrayGet; }
    };
    class ReturnStmt : public Node {
    public:
        std::unique_ptr<Node> expr;
        ReturnStmt(std::unique_ptr<Node> expr):expr(std::move(expr)) {}
        inline NodeType get_type() const override { return NodeType::Return; }
    };

    class Program {
    public:
        //function declaration (or const var declaration; TODO will be added later )
        std::vector<std::unique_ptr<FunctionDef>> declarations;
    };

    class IntLitExpr : public Node {
    public:
        const int64_t number;

        explicit IntLitExpr(int64_t val) : number(val) {}
        inline NodeType get_type() const override { return NodeType::IntLit; }
        [[nodiscard]] std::string to_str1() const override;
    };

    //actually any expression with identifier at first
    class VarExpr : public Node {
    public:
        VarType type;
        std::string name;

        explicit VarExpr(std::string name) : name(std::move(name)) {}
        std::string to_str1() const override;

        inline NodeType get_type() const override { return NodeType::Var; }

    };

    class IfStmt : public Node {
    public:
        std::unique_ptr<Node> expr;
        std::unique_ptr<Node> etrue = nullptr;
        std::unique_ptr<Node> efalse = nullptr;

        IfStmt(std::unique_ptr<Node> expr, std::unique_ptr<Node> etrue, std::unique_ptr<Node> efalse = nullptr)
                : expr(std::move(expr)), etrue(std::move(etrue)), efalse(std::move(efalse)) {}
        inline NodeType get_type() const override { return NodeType::If; }

    };

    class WhileStmt : public Node {
    public:
        std::unique_ptr<Node> expr;
        std::unique_ptr<Block> body;

        WhileStmt(std::unique_ptr<Node> expr, std::unique_ptr<Block> body) : expr(std::move(expr)),
                                                                             body(std::move(body)) {}
        inline NodeType get_type() const override { return NodeType::While; }

    };

    template<UnaryOpType type>
    class UnaryExpr : public Node {
    public:
        std::unique_ptr<Node> expr;

        explicit UnaryExpr(std::unique_ptr<Node> expr) : expr(std::move(expr)) {}

        std::string to_str1() const override { return "-(" + expr->to_str1() + ")"; }
        inline NodeType get_type() const override {
            switch (type) {
                case UnaryOpType::MINUS:
                    return NodeType::UnaryMinus;
            }
            throw std::runtime_error("not implemented");
        }


    };

    template<BinaryOpType type>
    class BinaryExpr : public Node {
    public:
        unique_ptr<Node> l, r;

        BinaryExpr(unique_ptr<Node> l, unique_ptr<Node> r) : l(std::move(l)), r(std::move(r)) {}
        inline NodeType get_type() const override {
            switch (type) {
                case BinaryOpType::ADD:
                    return NodeType::BinaryPlus;
                case BinaryOpType::MUL:
                    return NodeType::BinaryMul;
                case BinaryOpType::DIV:
                    return NodeType::BinaryDiv;
                case BinaryOpType::SUB:
                    return NodeType::BinaryMinus;
            }
            throw std::runtime_error("not implemented");
        }

        static std::string operatorString() {
            switch (type) {
                case BinaryOpType::ADD:
                    return "+";
                case BinaryOpType::SUB:
                    return "-";
                case BinaryOpType::MUL:
                    return "*";
                case BinaryOpType::DIV:
                    return "/";
                default:
                    throw std::invalid_argument("never see me");;
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
