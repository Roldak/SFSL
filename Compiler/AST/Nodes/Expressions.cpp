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

// TYPE SPECIFIER

TypeSpecifier::TypeSpecifier(Expression* specified, TypeNode* type) : _specified(specified), _type(type) {

}

TypeSpecifier::~TypeSpecifier() {

}

SFSL_AST_ON_VISIT_CPP(TypeSpecifier)

Expression* TypeSpecifier::getSpecified() const {
    return _specified;
}

TypeNode* TypeSpecifier::getTypeNode() const {
    return _type;
}

// MEMBER ACCESS

MemberAccess::MemberAccess(Expression* accessed, Identifier* member) : _accessed(accessed), _member(member) {

}

MemberAccess::~MemberAccess() {

}

SFSL_AST_ON_VISIT_CPP(MemberAccess)

Expression* MemberAccess::getAccessed() const {
    return _accessed;
}

Identifier* MemberAccess::getMember() const {
    return _member;
}

// TUPLE

Tuple::Tuple(const std::vector<Expression *> &exprs) : _exprs(exprs) {

}

Tuple::~Tuple() {

}

SFSL_AST_ON_VISIT_CPP(Tuple)

const std::vector<Expression*>& Tuple::getExpressions() {
    return _exprs;
}

// FUNCTION CREATION

FunctionCreation::FunctionCreation(Expression *args, Expression* body) : _args(args), _body(body) {

}

FunctionCreation::~FunctionCreation() {

}

SFSL_AST_ON_VISIT_CPP(FunctionCreation)

Expression* FunctionCreation::getArgs() const {
    return _args;
}

Expression* FunctionCreation::getBody() const {
    return _body;
}

// FUNCTION CALL

FunctionCall::FunctionCall(Expression* callee, Tuple* args)
    : _callee(callee), _args(args) {

}

FunctionCall::~FunctionCall() {

}

SFSL_AST_ON_VISIT_CPP(FunctionCall)

Expression* FunctionCall::getCallee() const {
    return _callee;
}

const std::vector<Expression*>& FunctionCall::getArgs() const {
    return _args->getExpressions();
}

Tuple* FunctionCall::getArgsTuple() const {
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
