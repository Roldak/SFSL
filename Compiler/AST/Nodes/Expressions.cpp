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

Expression::~Expression() {

}

SFSL_AST_ON_VISIT_CPP(Expression)

// BINARY EXPRESSION

BinaryExpression::BinaryExpression(Expression *lhs, Expression *rhs, Identifier *oper)
    : _lhs(lhs), _rhs(rhs), _oper(oper) {

}

BinaryExpression::~BinaryExpression() {

}

SFSL_AST_ON_VISIT_CPP(BinaryExpression)

Expression* BinaryExpression::getLhs() const {
    return _lhs;
}

Expression* BinaryExpression::getRhs() const {
    return _rhs;
}

Identifier* BinaryExpression::getOperator() const {
    return _oper;
}


// FUNCTION CALL

FunctionCall::FunctionCall(Expression *callee, const std::vector<Expression *> &args)
    : _callee(callee), _args(args) {

}

FunctionCall::~FunctionCall() {

}

SFSL_AST_ON_VISIT_CPP(FunctionCall)

Expression* FunctionCall::getCallee() const {
    return _callee;
}

const std::vector<Expression*>& FunctionCall::getArgs() const {
    return _args;
}

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
