//
//  Expressions.cpp
//  SFSL
//
//  Created by Romain Beguet on 17.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Expressions.h"
#include "../Visitors/ASTVisitor.h"

namespace sfsl {

namespace ast {

// EXPRESSION

Expression::Expression() {

}

Expression::~Expression() {

}

SFSL_AST_ON_VISIT_CPP(Expression)

// IDENTIFIER

Identifier::Identifier(const std::string& name) : _name(name) {

}

Identifier::~Identifier() {

}

const std::string& Identifier::getValue() const {
    return _name;
}

SFSL_AST_ON_VISIT_CPP(Identifier)

// INT LITTERAL

IntLitteral::IntLitteral(const sfsl_int_t value) : _value(value) {

}

IntLitteral::~IntLitteral() {

}

sfsl_int_t IntLitteral::getValue() const {
    return _value;
}

SFSL_AST_ON_VISIT_CPP(IntLitteral)

// REAL LITTERAL

RealLitteral::RealLitteral(const sfsl_real_t value) : _value(value) {

}

RealLitteral::~RealLitteral() {

}

sfsl_real_t RealLitteral::getValue() const {
    return _value;
}

SFSL_AST_ON_VISIT_CPP(RealLitteral)

}

}
