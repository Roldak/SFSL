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

// EXPRESSION STATEMENT

ExpressionStatement::ExpressionStatement(Expression *expr) : _expr(expr) {

}

ExpressionStatement::~ExpressionStatement() {

}

SFSL_AST_ON_VISIT_CPP(ExpressionStatement)

Expression *ExpressionStatement::getExpression() const {
    return _expr;
}

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

// ASSIGNMENT EXPRESSION

AssignmentExpression::AssignmentExpression(Expression *lhs, Expression *rhs)
    : _lhs(lhs), _rhs(rhs) {

}

AssignmentExpression::~AssignmentExpression() {

}

SFSL_AST_ON_VISIT_CPP(AssignmentExpression)

Expression* AssignmentExpression::getLhs() const {
    return _lhs;
}

Expression* AssignmentExpression::getRhs() const {
    return _rhs;
}

// TYPE SPECIFIER

TypeSpecifier::TypeSpecifier(Identifier* specified, TypeExpression* type) : _specified(specified), _type(type) {

}

TypeSpecifier::~TypeSpecifier() {

}

SFSL_AST_ON_VISIT_CPP(TypeSpecifier)

Identifier* TypeSpecifier::getSpecified() const {
    return _specified;
}

TypeExpression* TypeSpecifier::getTypeNode() const {
    return _type;
}

// BLOCK

Block::Block(const std::vector<Expression*>& stats) : _stats(stats) {

}

Block::~Block() {

}

SFSL_AST_ON_VISIT_CPP(Block)

const std::vector<Expression*>& Block::getStatements() const {
    return _stats;
}

// IF EXPRESSION

IfExpression::IfExpression(Expression* cond, Expression* then, Expression* els)
    : _cond(cond), _then(then), _else(els) {

}

IfExpression::~IfExpression() {

}

Expression *IfExpression::getCondition() const {
    return _cond;
}

Expression* IfExpression::getThen() const {
    return _then;
}

Expression* IfExpression::getElse() const {
    return _else;
}

SFSL_AST_ON_VISIT_CPP(IfExpression)

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

FunctionCreation::FunctionCreation(const std::string& name, Expression *args, Expression* body)
    : _name(name), _args(args), _body(body) {

}

FunctionCreation::~FunctionCreation() {

}

SFSL_AST_ON_VISIT_CPP(FunctionCreation)

const std::string& FunctionCreation::getName() const {
    return _name;
}

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

// THIS

This::This() {

}

This::~This() {

}

SFSL_AST_ON_VISIT_CPP(This)

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
