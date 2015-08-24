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
#include <vector>
#include <iterator>

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

/**
 * @brief An interface to keep track of
 * the used state of an object
 */
class UsageTrackable {
protected:

    UsageTrackable() : _used(false) {}

public:

    /**
     * @brief Sets the used state to the given value
     */
    void setUsed(bool val) {
        _used = val;
    }

    /**
     * @return True if the object has been used, otherwise false
     */
    bool isUsed() const {
        return _used;
    }

private:
    bool _used;
};

//(http://stackoverflow.com/questions/236129/split-a-string-in-c)
/**
 * @brief Splits the given strings according to a delimiter.
 * (Note : does not skip empty tokens.)
 * @param toFill The vector in which to add the parts
 * @param str The string to split
 * @param delim The delimiter for which to split the string
 * @return the number of parts that were split.
 */
inline size_t split(std::vector<std::string>& toFill, const std::string& str, char delim) {
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, delim)) {
        toFill.push_back(item);
    }
    return toFill.size();
}

template<typename Iterator>
/**
 * @brief An iterator adapter which returns
 * the second member of the underlying iterator's value
 */
class TakeSecond : public std::iterator<std::input_iterator_tag, typename Iterator::value_type> {
public:
    TakeSecond(const Iterator& it)
        : _it(it) { }

    typename Iterator::value_type::second_type operator *() const {
        return _it->second;
    }

    TakeSecond<Iterator>& operator ++() {
        ++_it;
        return *this;
    }

    bool operator ==(const TakeSecond<Iterator>& other) const {
        return _it == other._it;
    }

    bool operator !=(const TakeSecond<Iterator>& other) const {
        return _it != other._it;
    }

private:

    Iterator _it;
};

}

namespace chrutils {

/**
 * @return True if c is a new line ('\n' or '\r')
 */
inline bool isNewLine(char c) {
    return c == '\n' || c == '\r';
}

/**
 * @return True if c is a white space or equivalent
 */
inline bool isWhiteSpace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

/**
 * @return True if c is a digit
 */
inline bool isNumeric(char c) {
    return c >= '0' && c <= '9';
}

/**
 * @return True if c is a simple character (no special symbols)
 */
inline bool isCharacter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

/**
 * @return True if c is a quote symbol ('\"')
 */
inline bool isQuote(char c) {
    return c == '\"';
}

/**
 * @brief Maps the character given in parameter into its equivalent escaped,
 * e.g. if c = 'n', escapedChar('n') will yield c = '\n'
 *
 * @param c The character to escape (without the '\'). Will be modified by the function
 * @return True if the conversion was successful (c was a valid character that could be escaped)
 */
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

/**
 * @return True if c is a symbol ('+', '*', ';', '(', etc.)
 */
inline bool isSymbol(char c) {
    return (c == '=' || c == '>' || c == '<' || c == ',' || c == '&' || c == '|' ||
            c == '+' || c == '-' || c == '%' || c == '*' || c == '/' || c == '^' ||
            c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']' ||
            c == '!' || c == ':' || c == ';' || c == '?' || c == '@' || c == '.' ||
            c == '#');
}

}

// NOT IN THE utils NAMESPACE ANYMORE

/**
 * @brief The Boolean type adapted to the host architecture
 */
typedef bool sfsl_bool_t;

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
