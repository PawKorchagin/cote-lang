//
// Created by motya on 01.04.2025.
//

#ifndef COTE_MISC_H
#define COTE_MISC_H
#include <fstream>

inline std::istream NULL_STREAM(nullptr);

// SFINAE test
template <typename T>
class M_is_linked_ok
{
    typedef char one;
    struct two { char x[2]; };

    template <typename C> static one test1( decltype(&C::first) ) ;
    template <typename C> static two test1(...);

    template <typename C> static one test2( decltype(&C::last) ) ;
    template <typename C> static two test2(...);
public:
    enum { value = sizeof(test1<T>(0)) == sizeof(char) && sizeof(test2<T>(0)) == sizeof(char)  };
};



template<typename T, typename F>
void add_linked(T& obj, decltype(&T::last) nxt) {
    static_assert(M_is_linked_ok<T>::value && "class should have a pointer to the first item and "
                                              "last item for a linked list operation");
    obj->last->nxt = nxt;
    obj->last = obj->last->nxt;

}
#endif //COTE_MISC_H
