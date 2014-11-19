//
//  Utils.h
//  SFSL
//
//  Created by Romain Beguet on 13.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Utils__
#define __SFSL__Utils__

#include <sstream>

#define PTR_SIZE sizeof(void*)

namespace sfsl {

namespace utils {

template<unsigned int size>
struct AdaptedType   {
    typedef int adapted_int;
    typedef float adapted_real;
};
template<>
struct AdaptedType<4> {
    typedef int32_t adapted_int;
    typedef float adapted_real;
};
template<>
struct AdaptedType<8> {
    typedef int64_t adapted_int;
    typedef double adapted_real;
};

template<typename T>
/**
 * @param val The value to convert to String
 * @return the string representation of the given value
 */
inline std::string T_toString(T val) {
    std::ostringstream oss;
    oss << val;
    return oss.str();
}

template<typename T>
/**
 * @param val the string to convert into the given type
 * @return the value parsed from the input string
 */
inline T String_toT(const std::string& val) {
    std::istringstream oss(val);
    T res;
    oss >> res;
    return res;
}

}

namespace chrutils {

inline bool isWhiteSpace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

inline bool isNumeric(char c) {
    return c >= '0' && c <= '9';
}

inline bool isCharacter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

inline bool isQuote(char c) {
    return c == '\"';
}

inline bool escapedChar(char& c) {
    switch (c) {
    case '\'':
    case '\"':
    case '\\':
        break;
    case 'n':   c = '\n';
        break;
    case 't':   c = '\t';
        break;
    case '0':   c = '\0';
        break;
    default:
        return false;
    }
    return true;
}

inline bool isSymbol(char c) {
    return (c == '=' || c == '>' || c == '<' || c == ',' || c == '&' || c == '|' ||
            c == '+' || c == '-' || c == '%' || c == '*' || c == '/' || c == '^' ||
            c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']' ||
            c == '!' || c == ':' || c == ';' || c == '?' || c == '@' || c == '.');
}

}

/**
 * @brief The Integer type adapted to the host architecture (4 bytes or 8 bytes int)
 */
typedef utils::AdaptedType<PTR_SIZE>::adapted_int sfsl_int_t;

/**
 * @brief The Real type more or less adapted to the host architecture (float for 32bit archs and double for 64 archs)
 */
typedef utils::AdaptedType<PTR_SIZE>::adapted_real sfsl_real_t;

}

#endif
