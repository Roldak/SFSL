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

FunctionTypeDecl::FunctionTypeDecl(const std::vector<TypeExpression*>& typeArgs, const std::vector<TypeExpression*>& argTypes,
                                   TypeExpression* retType, TypeExpression* classEquivalent)
    : _typeArgs(typeArgs), _argTypes(argTypes), _retType(retType), _classEquivalent(classEquivalent) {

}

FunctionTypeDecl::~FunctionTypeDecl() {

}

SFSL_AST_ON_VISIT_CPP(FunctionTypeDecl)

const std::vector<TypeExpression*>& FunctionTypeDecl::getTypeArgs() const {
    return _typeArgs;
}

const std::vector<TypeExpression*>& FunctionTypeDecl::getArgTypes() const {
    return _argTypes;
}

TypeExpression* FunctionTypeDecl::getRetType() const {
    return _retType;
}

TypeExpression* FunctionTypeDecl::getClassEquivalent() const {
    return _classEquivalent;
}

TypeExpression* makeCallee(std::vector<std::string> path, CompCtx_Ptr ctx) {
    if (path.size() == 0) {
        return nullptr;
    } else if (path.size() == 1) {
        return ctx->memoryManager().New<TypeIdentifier>(path.back());
    } else {
        std::string id = path.back();
        path.pop_back();
        return ctx->memoryManager().New<TypeMemberAccess>(makeCallee(path, ctx), ctx->memoryManager().New<TypeIdentifier>(id));
    }
}

TypeExpression* FunctionTypeDecl::make(const std::vector<TypeExpression*>& typeArgs, const std::vector<TypeExpression*>& argTypes,
                                       TypeExpression* retType, const std::vector<std::string>& TCPath, CompCtx_Ptr ctx) {
    std::vector<TypeExpression*> args = argTypes;
    args.push_back(retType);

    TypeTuple* argsTuple = ctx->memoryManager().New<TypeTuple>(args);
    TypeExpression* callee = makeCallee(TCPath, ctx);
    TypeConstructorCall* classEq = ctx->memoryManager().New<TypeConstructorCall>(callee, argsTuple);

    return ctx->memoryManager().New<FunctionTypeDecl>(typeArgs, argTypes, retType, classEq);
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

const std::vector<TypeExpression*>& TypeTuple::getExpressions() const {
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

// TYPE PARAMETER

TypeParameter::TypeParameter(common::VARIANCE_TYPE varianceType, TypeIdentifier* specified, KindSpecifyingExpression* kind)
    : _varianceType(varianceType), _specified(specified), _kind(kind) {

}

TypeParameter::~TypeParameter() {

}

SFSL_AST_ON_VISIT_CPP(TypeParameter)

common::VARIANCE_TYPE TypeParameter::getVarianceType() const {
    return _varianceType;
}

TypeIdentifier* TypeParameter::getSpecified() const {
    return _specified;
}

KindSpecifyingExpression* TypeParameter::getKindNode() const {
    return _kind;
}

}

}
