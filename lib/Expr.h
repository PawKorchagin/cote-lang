//
// Created by motya on 28.03.2025.
//

#ifndef CRYPT_EXPR_H
#define CRYPT_EXPR_H
#include <string>

class Expr {
public:
    virtual std::string_view getType() = 0;
    virtual ~Expr() = default;
};

#endif //CRYPT_EXPR_H
