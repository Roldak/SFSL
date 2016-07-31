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

ExpressionStatement::ExpressionStatement(Expression* expr) : _expr(expr) {

}

ExpressionStatement::~ExpressionStatement() {

}

SFSL_AST_ON_VISIT_CPP(ExpressionStatement)

Expression *ExpressionStatement::getExpression() const {
    return _expr;
}

// DEFINE DECLARATION

DefineDecl::DefineDecl(Identifier* name, TypeExpression* typeSpecifier, Expression* value, bool isRedef, bool isExtern, bool isAbstract)
    : _name(name), _typeSpecifier(typeSpecifier), _value(value), _isRedef(isRedef), _isExtern(isExtern), _isAbstract(isAbstract) {

}

DefineDecl::~DefineDecl() {

}

SFSL_AST_ON_VISIT_CPP(DefineDecl)

Identifier* DefineDecl::getName() const {
    return _name;
}

TypeExpression*DefineDecl::getTypeSpecifier() const {
    return _typeSpecifier;
}

Expression* DefineDecl::getValue() const {
    return _value;
}

bool DefineDecl::isRedef() const {
    return _isRedef;
}

bool DefineDecl::isExtern() const {
    return _isExtern;
}

bool DefineDecl::isAbstract() const {
    return _isAbstract;
}

// TYPE DECLARATION

TypeDecl::TypeDecl(TypeIdentifier *id, TypeExpression *exp) : _name(id), _exp(exp) {

}

TypeDecl::~TypeDecl() {

}

SFSL_AST_ON_VISIT_CPP(TypeDecl)

TypeIdentifier* TypeDecl::getName() const {
    return _name;
}

TypeExpression* TypeDecl::getExpression() const {
    return _exp;
}

// ASSIGNMENT EXPRESSION

AssignmentExpression::AssignmentExpression(Expression* lhs, Expression* rhs)
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

SFSL_AST_ON_VISIT_CPP(IfExpression)

Expression *IfExpression::getCondition() const {
    return _cond;
}

Expression* IfExpression::getThen() const {
    return _then;
}

Expression* IfExpression::getElse() const {
    return _else;
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

FunctionCreation::FunctionCreation(const std::string& name, TypeTuple* typeArgs, Expression *args, Expression* body, TypeExpression* retType)
    : _name(name), _typeArgs(typeArgs), _args(args), _body(body), _retType(retType) {

}

FunctionCreation::~FunctionCreation() {

}

SFSL_AST_ON_VISIT_CPP(FunctionCreation)

const std::string& FunctionCreation::getName() const {
    return _name;
}

TypeTuple* FunctionCreation::getTypeArgs() const {
    return _typeArgs;
}

Expression* FunctionCreation::getArgs() const {
    return _args;
}

Expression* FunctionCreation::getBody() const {
    return _body;
}

TypeExpression*FunctionCreation::getReturnType() const {
    return _retType;
}

// FUNCTION CALL

FunctionCall::FunctionCall(Expression* callee, TypeTuple* typeArgs, Tuple* args)
    : _callee(callee), _typeArgs(typeArgs), _args(args) {

}

FunctionCall::~FunctionCall() {

}

SFSL_AST_ON_VISIT_CPP(FunctionCall)

Expression* FunctionCall::getCallee() const {
    return _callee;
}

TypeTuple* FunctionCall::getTypeArgsTuple() const {
    return _typeArgs;
}

Tuple* FunctionCall::getArgsTuple() const {
    return _args;
}

const std::vector<TypeExpression*>& FunctionCall::getTypeArgs() const {
    return _typeArgs->getExpressions();
}

const std::vector<Expression*>& FunctionCall::getArgs() const {
    return _args->getExpressions();
}

// INSTANTIATION

Instantiation::Instantiation(TypeExpression* instantiated) : _instantiated(instantiated) {

}

Instantiation::~Instantiation() {

}

SFSL_AST_ON_VISIT_CPP(Instantiation)

TypeExpression* Instantiation::getInstantiatedExpression() const {
    return _instantiated;
}

// IDENTIFIER

Identifier::Identifier(const std::string& name) : _name(name) {

}

Identifier::~Identifier() {

}

SFSL_AST_ON_VISIT_CPP(Identifier)

const std::string& Identifier::getValue() const {
    return _name;
}

// THIS

This::This() {

}

This::~This() {

}

SFSL_AST_ON_VISIT_CPP(This)

// BOOL Literal

BoolLiteral::BoolLiteral(const sfsl_bool_t value) : _value(value) {

}

BoolLiteral::~BoolLiteral() {

}

SFSL_AST_ON_VISIT_CPP(BoolLiteral)

sfsl_bool_t BoolLiteral::getValue() const {
    return _value;
}

// INT Literal

IntLiteral::IntLiteral(const sfsl_int_t value) : _value(value) {

}

IntLiteral::~IntLiteral() {

}

sfsl_int_t IntLiteral::getValue() const {
    return _value;
}

SFSL_AST_ON_VISIT_CPP(IntLiteral)

// REAL Literal

RealLiteral::RealLiteral(const sfsl_real_t value) : _value(value) {

}

RealLiteral::~RealLiteral() {

}

SFSL_AST_ON_VISIT_CPP(RealLiteral)

sfsl_real_t RealLiteral::getValue() const {
    return _value;
}

// STRING Literal

StringLiteral::StringLiteral(const std::string& value) : _value(value) {

}

StringLiteral::~StringLiteral() {

}

SFSL_AST_ON_VISIT_CPP(StringLiteral)

const std::string& StringLiteral::getValue() const {
    return _value;
}

}

}
