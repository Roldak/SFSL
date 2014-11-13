//
//  Utils.h
//  SFSL
//
//  Created by Romain Beguet on 13.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Lexer__
#define __SFSL__Lexer__

#include <sstream>

#define PTR_SIZE sizeof(void*)

namespace sfsl {

namespace utils {

template<unsigned int size> struct AdaptedType   {
    typedef int adapted_int;
    typedef float adapted_real;
};
template<>                  struct AdaptedType<4> {
    typedef int32_t adapted_int;
    typedef float adapted_real;
};
template<>                  struct AdaptedType<8> {
    typedef int64_t adapted_int;
    typedef double adapted_real;
};

template<typename T>
inline std::string T_toString(T val) {
    std::ostringstream oss;
    oss << val;
    return oss.str();
}

template<typename T>
inline T String_toT(const std::string& val) {
    std::istringstream oss(val);
    T res;
    oss >> res;
    return res;
}

}

typedef utils::AdaptedType<PTR_SIZE>::adapted_int sfsl_int_t;
typedef utils::AdaptedType<PTR_SIZE>::adapted_real sfsl_real_t;

}

#endif
