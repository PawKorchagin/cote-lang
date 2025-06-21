//
// Created by motya on 20.06.2025.
//

#ifndef CRYPT_TYPE_H
#define CRYPT_TYPE_H

#include <cstdint>

namespace jit {

    struct Type {
        union {
            uint64_t bits;
            struct {
                uint32_t minfo;
                uint32_t mdata;
            };
        };

    };
}

#endif //CRYPT_TYPE_H
