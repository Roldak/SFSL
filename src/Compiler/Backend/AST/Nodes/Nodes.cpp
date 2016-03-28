//
//  Nodes.cpp
//  SFSL
//
//  Created by Romain Beguet on 28.03.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#include "Nodes.h"
#include "../Visitors/BASTVisitor.h"

namespace sfsl {

namespace bast {

// DEFINITION

Definition::Definition(const std::string& name) : _name(name) {

}

Definition::~Definition() {

}

const std::string& Definition::getName() const {
    return _name;
}

// CLASSDEF METHOD

ClassDef::Method::Method(const std::string& name, size_t argCount)
    : _name(name), _argCount(argCount), _body(nullptr) {

}

ClassDef::Method::Method(const std::string& name, size_t argCount, BASTNode* body)
    : _name(name), _argCount(argCount), _body(body) {

}

const std::string& ClassDef::Method::getName() const {
    return _name;
}

size_t ClassDef::Method::getArgCount() const {
    return _argCount;
}

BASTNode* ClassDef::Method::getMethodBody() const {
    return _body;
}

// CLASSDEF

ClassDef::ClassDef(const std::string& name, size_t fieldCount, std::vector<ClassDef::Method>& methods)
    : Definition(name), _fieldCount(fieldCount), _methods(methods) {

}

ClassDef::~ClassDef() {

}

size_t ClassDef::getFieldCount() const {
    return _fieldCount;
}

const std::vector<ClassDef::Method>& ClassDef::getMethods() const {
    return _methods;
}

// GLOBALDEF


GlobalDef::GlobalDef(const std::string& name, BASTNode* body)
    : Definition(name), _body(body) {

}

GlobalDef::~GlobalDef() {

}

BASTNode* GlobalDef::getBody() const {
    return _body;
}

// PROGRAM

Program::Program(const std::vector<ClassDef>& classes, const std::vector<GlobalDef>& globals)
    : _classes(classes), _globals(globals) {

}

Program::~Program() {

}

SFSL_BAST_ON_VISIT_CPP(Program)

const std::vector<ClassDef>& Program::getClasses() const {
    return _classes;
}

const std::vector<GlobalDef>&Program::getGlobals() const {
    return _globals;
}

// EXPRESSION

Expression::~Expression() {

}

SFSL_BAST_ON_VISIT_CPP(Expression)

// BLOCK

Block::Block(const std::vector<Expression*>& stats) : _stats(stats) {

}

Block::~Block() {

}

SFSL_BAST_ON_VISIT_CPP(Block)


const std::vector<Expression*>& Block::getStatements() const {
    return _stats;
}

// IF EXPRESSION

IfExpression::IfExpression(Expression* cond, Expression* then, Expression* els)
    : _cond(cond), _then(then), _else(els) {

}

IfExpression::~IfExpression() {

}

SFSL_BAST_ON_VISIT_CPP(IfExpression)

Expression* IfExpression::getCondition() const {
    return _cond;
}

Expression* IfExpression::getThen() const {
    return _then;
}

Expression* IfExpression::getElse() const {
    return _else;
}

// FIELD ACCESS

FieldAccess::FieldAccess(Expression* accessed, size_t fieldId)
    : _accessed(accessed), _fieldId(fieldId) {

}

FieldAccess::~FieldAccess() {

}

SFSL_BAST_ON_VISIT_CPP(FieldAccess)

Expression* FieldAccess::getAccessed() const {
    return _accessed;
}

size_t FieldAccess::getFieldId() const {
    return _fieldId;
}

// FIELD ASSIGNMENT EXPRESSION

FieldAssignmentExpression::FieldAssignmentExpression(Expression* accessed, size_t fieldId, Expression* value)
    : _accessed(accessed), _fieldId(fieldId), _value(value) {

}

FieldAssignmentExpression::~FieldAssignmentExpression() {

}

SFSL_BAST_ON_VISIT_CPP(FieldAssignmentExpression)

Expression* FieldAssignmentExpression::getAccessed() const {
    return _accessed;
}

size_t FieldAssignmentExpression::getFieldId() const {
    return _fieldId;
}

Expression* FieldAssignmentExpression::getValue() const {
    return _value;
}

// METHOD CALL

MethodCall::MethodCall(Expression* callee, size_t virtualId, const std::vector<Expression*>& args)
    : _callee(callee), _virtualId(virtualId), _args(args) {

}

MethodCall::~MethodCall() {

}

SFSL_BAST_ON_VISIT_CPP(MethodCall)

Expression* MethodCall::getCallee() const {
    return _callee;
}

size_t MethodCall::getVirtualId() const {
    return _virtualId;
}

const std::vector<Expression*>& MethodCall::getArgs() const {
    return _args;
}

// DEF IDENTIFIER

DefIdentifier::DefIdentifier(const std::string& name) : _name(name) {

}

DefIdentifier::~DefIdentifier() {

}

SFSL_BAST_ON_VISIT_CPP(DefIdentifier)

const std::string&DefIdentifier::getValue() const {
    return _name;
}

// VAR IDENTIFIER

VarIdentifier::VarIdentifier(size_t localId) : _localId(localId) {

}

VarIdentifier::~VarIdentifier() {

}

SFSL_BAST_ON_VISIT_CPP(VarIdentifier)

size_t VarIdentifier::getLocalId() const {
    return _localId;
}

// VAR ASSIGNMENT EXPRESSION

VarAssignmentExpression::VarAssignmentExpression(size_t localId, Expression* value)
    : _localId(localId), _value(value) {

}

VarAssignmentExpression::~VarAssignmentExpression() {

}

SFSL_BAST_ON_VISIT_CPP(VarAssignmentExpression)

size_t VarAssignmentExpression::getAssignedVarLocalId() const {
    return _localId;
}

Expression* VarAssignmentExpression::getValue() const {
    return _value;
}

// INSTANTIATION

Instantiation::Instantiation(DefIdentifier* defId) : _defId(defId) {

}

Instantiation::~Instantiation() {

}

SFSL_BAST_ON_VISIT_CPP(Instantiation)

DefIdentifier* Instantiation::getClassId() const {
    return _defId;
}

// BOOL LITTERAL

BoolLitteral::BoolLitteral(const sfsl_bool_t value) : _value(value) {

}

BoolLitteral::~BoolLitteral() {

}

SFSL_BAST_ON_VISIT_CPP(BoolLitteral)

sfsl_bool_t BoolLitteral::getValue() const {
    return _value;
}

// INT LITTERAL

IntLitteral::IntLitteral(const sfsl_int_t value) : _value(value) {

}

IntLitteral::~IntLitteral() {

}

sfsl_int_t IntLitteral::getValue() const {
    return _value;
}

SFSL_BAST_ON_VISIT_CPP(IntLitteral)

// REAL LITTERAL

RealLitteral::RealLitteral(const sfsl_real_t value) : _value(value) {

}

RealLitteral::~RealLitteral() {

}

SFSL_BAST_ON_VISIT_CPP(RealLitteral)

sfsl_real_t RealLitteral::getValue() const {
    return _value;
}

// STRING LITTERAL

StringLitteral::StringLitteral(const std::string& value) : _value(value) {

}

StringLitteral::~StringLitteral() {

}

SFSL_BAST_ON_VISIT_CPP(StringLitteral)

const std::string& StringLitteral::getValue() const {
    return _value;
}

}

}
