//
// Created by Георгий on 01.04.2025.
//

#ifndef CONCEPT_H
#define CONCEPT_H

namespace ast {
    // to requires different types
    template<typename T>
    concept is_commutative = std::same_as<T, const AddExpr*> || std::same_as<T, const MulExpr*>;

    template<typename T>
    concept is_binary_operation = (is_commutative<T> || std::same_as<T, const SubExpr*> || std::same_as<T, const DivExpr*>);
}

#endif //CONCEPT_H
