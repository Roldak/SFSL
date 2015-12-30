//
//  TypeExpressions.cpp
//  SFSL
//
//  Created by Romain Beguet on 23.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "TypeExpressions.h"
#include "../Visitors/ASTImplicitVisitor.h"

namespace sfsl {

namespace ast {

// TYPE EXPRESSION

TypeExpression::~TypeExpression() {

}

SFSL_AST_ON_VISIT_CPP(TypeExpression)

// CLASS DECLARATION

ClassDecl::ClassDecl(const std::string& name,
                     TypeExpression* parent,
                     const std::vector<TypeDecl*>& tdecls,
                     const std::vector<TypeSpecifier*>& fields,
                     const std::vector<DefineDecl*>& defs,
                     bool isAbstract)

    : _name(name), _parent(parent), _tdecls(tdecls), _fields(fields), _defs(defs), _isAbstract(isAbstract)
{

}

ClassDecl::~ClassDecl() {

}

SFSL_AST_ON_VISIT_CPP(ClassDecl)

const std::string& ClassDecl::getName() const {
    return _name;
}

TypeExpression* ClassDecl::getParent() const {
    return _parent;
}

const std::vector<TypeDecl*>& ClassDecl::getTypeDecls() const {
    return _tdecls;
}

const std::vector<TypeSpecifier*>& ClassDecl::getFields() const {
    return _fields;
}

const std::vector<DefineDecl*>& ClassDecl::getDefs() const{
    return _defs;
}

bool ClassDecl::isAbstract() const {
    return _isAbstract;
}

// FUNCTION TYPE DECLARATION

FunctionTypeDecl::FunctionTypeDecl(const std::vector<TypeExpression*>& argTypes, TypeExpression* retType)
    : _argTypes(argTypes), _retType(retType) {

}

FunctionTypeDecl::~FunctionTypeDecl() {

}

SFSL_AST_ON_VISIT_CPP(FunctionTypeDecl)

const std::vector<TypeExpression*>& FunctionTypeDecl::getArgTypes() const {
    return _argTypes;
}

TypeExpression* FunctionTypeDecl::getRetType() const {
    return _retType;
}

// TYPE MEMBER ACCESS

TypeMemberAccess::TypeMemberAccess(TypeExpression* accessed, TypeIdentifier* member) : _accessed(accessed), _member(member) {

}

TypeMemberAccess::~TypeMemberAccess() {

}

SFSL_AST_ON_VISIT_CPP(TypeMemberAccess)

TypeExpression* TypeMemberAccess::getAccessed() const {
    return _accessed;
}

TypeIdentifier* TypeMemberAccess::getMember() const {
    return _member;
}

// TYPE TUPLE

TypeTuple::TypeTuple(const std::vector<TypeExpression*>& exprs) : _exprs(exprs) {

}

TypeTuple::~TypeTuple() {

}

SFSL_AST_ON_VISIT_CPP(TypeTuple)

const std::vector<TypeExpression*>& TypeTuple::getExpressions() {
    return _exprs;
}

// TYPE CONSTRUCTOR CREATION

TypeConstructorCreation::TypeConstructorCreation(const std::string &name, TypeExpression* args, TypeExpression* body)
    : _name(name), _args(args), _body(body) {

}

TypeConstructorCreation::~TypeConstructorCreation() {

}

SFSL_AST_ON_VISIT_CPP(TypeConstructorCreation)

const std::string& TypeConstructorCreation::getName() const {
    return _name;
}

TypeExpression* TypeConstructorCreation::getArgs() const {
    return _args;
}

TypeExpression* TypeConstructorCreation::getBody() const {
    return _body;
}

// TYPE CONSTRUCTOR CALL

TypeConstructorCall::TypeConstructorCall(TypeExpression* callee, TypeTuple* args)
    : _callee(callee), _args(args) {

}

TypeConstructorCall::~TypeConstructorCall() {

}

SFSL_AST_ON_VISIT_CPP(TypeConstructorCall)

TypeExpression* TypeConstructorCall::getCallee() const {
    return _callee;
}

const std::vector<TypeExpression*>& TypeConstructorCall::getArgs() const {
    return _args->getExpressions();
}

TypeTuple* TypeConstructorCall::getArgsTuple() const {
    return _args;
}

// TYPE IDENTIFIER

TypeIdentifier::TypeIdentifier(const std::string &name) : _name(name) {

}

TypeIdentifier::~TypeIdentifier() {

}

SFSL_AST_ON_VISIT_CPP(TypeIdentifier)

const std::string& TypeIdentifier::getValue() const {
    return _name;
}

// TYPE TO BE INFERRED

TypeToBeInferred::TypeToBeInferred() {

}

TypeToBeInferred::~TypeToBeInferred() {

}

SFSL_AST_ON_VISIT_CPP(TypeToBeInferred)

// KIND SPECIFIER

KindSpecifier::KindSpecifier(TypeIdentifier* specified, KindSpecifyingExpression* kind) : _specified(specified), _kind(kind) {

}

KindSpecifier::~KindSpecifier() {

}

SFSL_AST_ON_VISIT_CPP(KindSpecifier)

TypeIdentifier* KindSpecifier::getSpecified() const {
    return _specified;
}

KindSpecifyingExpression* KindSpecifier::getKindNode() const {
    return _kind;
}

}

}
