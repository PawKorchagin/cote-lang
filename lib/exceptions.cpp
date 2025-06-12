//
// Created by motya on 14.04.2025.
//
#include "exceptions.h"
#include "lexer.h"
#include <vector>
#include <string>
#include <stdexcept>

namespace parser {
    bool panic_mode = false;
    std::vector<std::string> error_log;

    void init_exceptions() {
        error_log.clear();
        panic_mode = false;
    }

    std::vector<std::string> get_errors() {
        return error_log;
    }

    bool is_panic() { return panic_mode; }

    void panic() {
        while (true) {
            switch (cur.token) {
                case TOKEN_EOF:
                    return;
                case TOKEN_FN:
                    if (get_tok() == TOKEN_IDENTIFIER) {
                        panic_mode = false;
                        return;
                    }
                default:
                    get_tok();
            }
        }
        //TODO
    }

    std::nullptr_t parser_throws(const std::string &message) {
        if (panic_mode) return nullptr;
        panic_mode = true;
        throw std::runtime_error(std::to_string(cur.lines) + ":" + std::to_string(cur.cnt - 1) + ": " + message);
    }

    std::string error_msg(const std::string &text) {
        return "expected " + text + " but found " + token_to_string(static_cast<TokenInfo>(cur.token), cur.identifier);
    }
}
