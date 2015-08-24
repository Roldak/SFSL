//
//  Litterals.cpp
//  SFSL
//
//  Created by Romain Beguet on 13.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Litterals.h"

namespace sfsl {

namespace tok {

// BOOLS

BoolLitteral::BoolLitteral(sfsl_bool_t value) : _value(value) {

}

BoolLitteral::~BoolLitteral() {

}

TOK_TYPE BoolLitteral::getTokenType() const {
    return TOK_BOOL_LIT;
}

std::string BoolLitteral::toString() const {
    return _value ? "true" : "false";
}

sfsl_bool_t BoolLitteral::getValue() const {
    return _value;
}

// INTS

IntLitteral::IntLitteral(sfsl_int_t value)  : _value(value) {

}

IntLitteral::~IntLitteral() {

}

TOK_TYPE IntLitteral::getTokenType() const {
    return TOK_INT_LIT;
}

std::string IntLitteral::toString() const {
    return utils::T_toString(_value);
}

sfsl_int_t IntLitteral::getValue() const {
    return _value;
}

// REALS

RealLitteral::RealLitteral(sfsl_real_t value) : _value(value) {

}

RealLitteral::~RealLitteral() {

}

TOK_TYPE RealLitteral::getTokenType() const {
    return TOK_REAL_LIT;
}

std::string RealLitteral::toString() const {
    return utils::T_toString(_value);
}

sfsl_real_t RealLitteral::getValue() const {
    return _value;
}

// STRS

StringLitteral::StringLitteral(const std::string &value) : _value(value) {

}

StringLitteral::~StringLitteral() {

}

TOK_TYPE StringLitteral::getTokenType() const {
    return TOK_STR_LIT;
}

std::string StringLitteral::toString() const {
    return "\"" + _value + "\"";
}

std::string StringLitteral::getValue() const {
    return _value;
}



}

}
