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

SFSL_BAST_ON_VISIT_CPP(Definition)

const std::string& Definition::getName() const {
    return _name;
}

// METHOD

MethodDef::MethodDef(const std::string& name, size_t varCount)
    : Definition(name), _varCount(varCount), _body(nullptr) {

}

MethodDef::MethodDef(const std::string& name, size_t varCount, BASTNode* body)
    : Definition(name), _varCount(varCount), _body(body) {

}

MethodDef::~MethodDef() {

}

SFSL_BAST_ON_VISIT_CPP(MethodDef)

size_t MethodDef::getVarCount() const {
    return _varCount;
}

BASTNode* MethodDef::getMethodBody() const {
    return _body;
}

// CLASSDEF

ClassDef::ClassDef(const std::string& name, size_t fieldCount, DefIdentifier* parent, const std::vector<DefIdentifier*>& methods)
    : Definition(name), _fieldCount(fieldCount), _parent(parent), _methods(methods) {

}

ClassDef::~ClassDef() {

}

SFSL_BAST_ON_VISIT_CPP(ClassDef)

size_t ClassDef::getFieldCount() const {
    return _fieldCount;
}

DefIdentifier* ClassDef::getParent() const {
    return _parent;
}

const std::vector<DefIdentifier*>& ClassDef::getMethods() const {
    return _methods;
}

// GLOBALDEF


GlobalDef::GlobalDef(const std::string& name) : Definition(name), _body(nullptr) {

}

GlobalDef::GlobalDef(const std::string& name, BASTNode* body) : Definition(name), _body(body) {

}

GlobalDef::~GlobalDef() {

}

SFSL_BAST_ON_VISIT_CPP(GlobalDef)

BASTNode* GlobalDef::getBody() const {
    return _body;
}

// PROGRAM

Program::Program(const std::vector<Definition*>& visibleDefs, const std::vector<Definition*>& hiddenDefs)
    : _visibleDefs(visibleDefs), _hiddenDefs(hiddenDefs) {

}

Program::~Program() {

}

SFSL_BAST_ON_VISIT_CPP(Program)

const std::vector<Definition*>& Program::getVisibleDefinitions() const {
    return _visibleDefs;
}

const std::vector<Definition*>& Program::getHiddenDefinitions() const {
    return _hiddenDefs;
}

// EXPRESSION

Expression::~Expression() {

}

SFSL_BAST_ON_VISIT_CPP(Expression)

// BLOCK

Block::Block(const std::vector<Expression*>& stats) : _stats(stats) {
    if (stats.size() == 0) {
        throw common::CompilationFatalError("Cannot create block containing zero statements");
    }
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

// UNIT Literal

UnitLiteral::UnitLiteral() {

}

UnitLiteral::~UnitLiteral() {

}

SFSL_BAST_ON_VISIT_CPP(UnitLiteral)

// BOOL Literal

BoolLiteral::BoolLiteral(const sfsl_bool_t value) : _value(value) {

}

BoolLiteral::~BoolLiteral() {

}

SFSL_BAST_ON_VISIT_CPP(BoolLiteral)

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

SFSL_BAST_ON_VISIT_CPP(IntLiteral)

// REAL Literal

RealLiteral::RealLiteral(const sfsl_real_t value) : _value(value) {

}

RealLiteral::~RealLiteral() {

}

SFSL_BAST_ON_VISIT_CPP(RealLiteral)

sfsl_real_t RealLiteral::getValue() const {
    return _value;
}

// STRING Literal

StringLiteral::StringLiteral(const std::string& value) : _value(value) {

}

StringLiteral::~StringLiteral() {

}

SFSL_BAST_ON_VISIT_CPP(StringLiteral)

const std::string& StringLiteral::getValue() const {
    return _value;
}

}

}
