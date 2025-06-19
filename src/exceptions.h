//
// Created by motya on 14.04.2025.
//

#ifndef COTE_EXCEPTIONS_H
#define COTE_EXCEPTIONS_H

#include <vector>
#include <string>

namespace parser {
    std::nullptr_t parser_throws(const std::string &message);

    std::string error_msg(const std::string &text);
    void init_exceptions();

//stops at ; or declaration or statement
    void panic();
    bool is_panic();

    std::vector<std::string> get_errors();

}

#endif //COTE_EXCEPTIONS_H
