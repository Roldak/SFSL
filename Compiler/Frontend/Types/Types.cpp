//
//  Types.cpp
//  SFSL
//
//  Created by Romain Beguet on 24.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Types.h"
#include "../AST/Nodes/TypeExpressions.h"
#include "../AST/Visitors/ASTTypeCreator.h"
#include "../AST/Visitors/ASTTypeIdentifier.h"
#include "../AST/Visitors/ASTSymbolExtractor.h"

namespace sfsl {

namespace type {

// TYPE NOT YET DEFINED

class TypeNotYetDefined : public Type {
public:

    TypeNotYetDefined() {}

    virtual ~TypeNotYetDefined() {}

    virtual TYPE_KIND getTypeKind() const {
        return TYPE_NYD;
    }

    virtual bool isSubTypeOf(const Type*) const {
        return false;
    }

    virtual Type* applyEnv(const SubstitutionTable&, CompCtx_Ptr&) const {
        return Type::NotYetDefined();
    }

    virtual std::string toString() const {
        return "<not yet defined>";
    }
};

// TYPE

Type::Type(const SubstitutionTable &substitutionTable) : _subTable(substitutionTable) {

}

Type::~Type() {

}

const static SubstitutionTable DefaultTable = {};

Type* Type::applied(CompCtx_Ptr& ctx) const {
    return applyEnv(DefaultTable, ctx);
}

std::string Type::toString() const {
    std::string toRet = "";
    if (!_subTable.empty()) {
        toRet += "[";
        for (const auto& pair : _subTable) {
            toRet += pair.first->toString() + "=>" + pair.second->toString() + ", ";
        }
        toRet = toRet.substr(0, toRet.size() - 2) + "]";
    }
    return toRet;
}

const SubstitutionTable& Type::getSubstitutionTable() const {
    return _subTable;
}

Type* Type::NotYetDefined() {
    static TypeNotYetDefined nyd;
    return &nyd; // all we want is a unique memory area
}

Type* Type::findSubstitution(const SubstitutionTable& table, Type* toFind) {
    auto found = table.find(toFind);
    while (found != table.end()) {
        toFind = found->second;
        found = table.find(toFind);
    }
    return toFind;
}

void Type::applyEnvHelper(const SubstitutionTable& env, SubstitutionTable& to) {
    for (auto& pair : to) {
        pair.second = findSubstitution(env, pair.second);
    }
}

// PROPER TYPE

ProperType::ProperType(ast::ClassDecl* clss, const SubstitutionTable& substitutionTable)
    : Type(substitutionTable), _class(clss) {

}

ProperType::~ProperType() {

}

TYPE_KIND ProperType::getTypeKind() const { return TYPE_PROPER; }

bool ProperType::isSubTypeOf(const Type* other) const {
    if (ProperType* objother = getIf<ProperType>(other)) {
        if (_class == objother->_class) { // TODO : change that when inheritance is supported.
            const SubstitutionTable& osub = objother->getSubstitutionTable();
            for (const auto& pair : _subTable) {
                const auto& subpair = osub.find(pair.first);
                if (!subpair->second->isSubTypeOf(pair.second)) { // TODO support contravariance maybe?
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

std::string ProperType::toString() const {
    return _class->getName() + Type::toString();
}

Type* ProperType::applyEnv(const SubstitutionTable& env, CompCtx_Ptr& ctx) const {
    if (env.empty()) return (Type*)this;

    SubstitutionTable table = _subTable;
    applyEnvHelper(env, table);
    return ctx.get()->memoryManager().New<ProperType>(_class, table);
}

ast::ClassDecl* ProperType::getClass() const {
    return _class;
}

// FUNCTION TYPE

FunctionType::FunctionType(const std::vector<Type*>& argTypes, Type* retType, ast::ClassDecl* clss, const SubstitutionTable& substitutionTable)
    : ProperType(clss, substitutionTable), _argTypes(argTypes), _retType(retType) {

}

FunctionType::~FunctionType() {

}

TYPE_KIND FunctionType::getTypeKind() const {
    return TYPE_FUNCTION;
}

bool FunctionType::isSubTypeOf(const Type* other) const {
    if (FunctionType* f = getIf<FunctionType>(other)) {
        const std::vector<Type*>& oArgTypes = f->getArgTypes();
        const Type* oRetType = f->getRetType();

        if (_argTypes.size() != oArgTypes.size()) {
            return false;
        }

        for (size_t i = 0; i < _argTypes.size(); ++i) {
            if (!_argTypes[i]->isSubTypeOf(oArgTypes[i])) {
                return false;
            }
        }

        return oRetType->isSubTypeOf(_retType);
    }

    return false;
}

std::string FunctionType::toString() const {
    std::string toRet = "(";

    if (_argTypes.size() > 0) {
        for (size_t i = 0; i < _argTypes.size() - 1; ++i) {
            toRet += _argTypes[i]->toString() + ", ";
        }
        toRet += _argTypes.back()->toString();
    }

    return toRet + ")->" + _retType->toString();
}

Type* FunctionType::applyEnv(const SubstitutionTable& env, CompCtx_Ptr& ctx) const {
    std::vector<Type*> newArgTypes;
    Type* newRetType;

    for (Type* t : _argTypes) {
        newArgTypes.push_back(findSubstitution(env, t)->applyEnv(env, ctx));
    }

    newRetType = findSubstitution(env, _retType)->applyEnv(env, ctx);

    return ctx->memoryManager().New<FunctionType>(newArgTypes, newRetType, _class, _subTable);
}

const std::vector<Type*>& FunctionType::getArgTypes() const {
    return _argTypes;
}

Type* FunctionType::getRetType() const {
    return _retType;
}

// METHOD TYPE

MethodType::MethodType(ast::ClassDecl* owner, const std::vector<Type*>& argTypes, Type* retType, const SubstitutionTable& substitutionTable)
    : Type(substitutionTable), _owner(owner), _argTypes(argTypes), _retType(retType) {

}

MethodType::~MethodType() {

}

TYPE_KIND MethodType::getTypeKind() const {
    return TYPE_METHOD;
}

bool MethodType::isSubTypeOf(const Type* other) const {
    if (FunctionType* f = getIf<FunctionType>(other)) {
        const std::vector<Type*>& oArgTypes = f->getArgTypes();
        const Type* oRetType = f->getRetType();

        if (_argTypes.size() != oArgTypes.size()) {
            return false;
        }

        for (size_t i = 0; i < _argTypes.size(); ++i) {
            if (!_argTypes[i]->isSubTypeOf(oArgTypes[i])) {
                return false;
            }
        }

        return oRetType->isSubTypeOf(_retType);
    }

    return false;
}

std::string MethodType::toString() const {
    std::string toRet = "([" + _owner->getName() + "]";

    for (size_t i = 0; i < _argTypes.size(); ++i) {
        toRet += ", " + _argTypes[i]->toString();
    }

    return toRet + ")->" + _retType->toString();
}

Type* MethodType::applyEnv(const SubstitutionTable& env, CompCtx_Ptr& ctx) const {
    std::vector<Type*> newArgTypes;
    Type* newRetType;

    for (Type* t : _argTypes) {
        newArgTypes.push_back(findSubstitution(env, t)->applyEnv(env, ctx));
    }

    newRetType = findSubstitution(env, _retType)->applyEnv(env, ctx);

    return ctx->memoryManager().New<MethodType>(_owner, newArgTypes, newRetType, _subTable);
}

ast::ClassDecl *MethodType::getOwner() const {
    return _owner;
}

const std::vector<Type*>& MethodType::getArgTypes() const {
    return _argTypes;
}

Type* MethodType::getRetType() const {
    return _retType;
}

// TYPE CONSTRUCTOR

TypeConstructorType::TypeConstructorType(ast::TypeConstructorCreation*typeConstructor, const SubstitutionTable& substitutionTable)
    : Type(substitutionTable), _typeConstructor(typeConstructor) {

}

TypeConstructorType::~TypeConstructorType() {

}

TYPE_KIND TypeConstructorType::getTypeKind() const {
    return TYPE_CONSTRUCTOR_TYPE;
}

bool TypeConstructorType::isSubTypeOf(const Type* other) const {
    return this == other;
}

std::string TypeConstructorType::toString() const {
    return _typeConstructor->getName() + Type::toString();
}

Type* TypeConstructorType::applyEnv(const SubstitutionTable& env, CompCtx_Ptr& ctx) const {
    if (env.empty()) return (Type*)this;

    SubstitutionTable table = _subTable;
    applyEnvHelper(env, table);
    return ctx.get()->memoryManager().New<TypeConstructorType>(_typeConstructor, table);
}

ast::TypeConstructorCreation* TypeConstructorType::getTypeConstructor() const {
    return _typeConstructor;
}

// CONSTRUCTOR APPLY TYPE

ConstructorApplyType::ConstructorApplyType(Type* callee, const std::vector<Type*>& args,
                                           const common::Positionnable& pos,
                                           const SubstitutionTable& substitutionTable)
    : Type(substitutionTable), _callee(callee), _args(args), _pos(pos) {

}

ConstructorApplyType::~ConstructorApplyType() {

}

TYPE_KIND ConstructorApplyType::getTypeKind() const {
    return TYPE_CONSTRUCTOR_APPLY;
}

bool ConstructorApplyType::isSubTypeOf(const Type* other) const {
    return false;
}

std::string ConstructorApplyType::toString() const {
    std::string toRet = _callee->toString() + "[";
    for (size_t i = 0; i < _args.size(); ++i) {
        toRet += _args[i]->toString();
        if (i != _args.size() - 1) {
            toRet += ", ";
        }
    }
    return toRet + "]";
}

Type* ConstructorApplyType::applyEnv(const SubstitutionTable& env, CompCtx_Ptr& ctx) const {
    TypeConstructorType* ctr = static_cast<TypeConstructorType*>(findSubstitution(env, _callee)->applyEnv(env, ctx));

    ast::TypeExpression* expr = ctr->getTypeConstructor()->getArgs();
    std::vector<ast::TypeExpression*> params;

    if (ast::isNodeOfType<ast::TypeTuple>(expr, ctx)) { // form is `[] => ...` or `[exp, exp] => ...`, ...
        params = static_cast<ast::TypeTuple*>(expr)->getExpressions();
    } else { // form is `exp => ...` or `[exp] => ...`
        params.push_back(expr);
    }

    SubstitutionTable subs;

    for (size_t i = 0; i < params.size(); ++i) {
        // can only be a TypeSymbol
        sym::TypeSymbol* param = static_cast<sym::TypeSymbol*>(ast::ASTSymbolExtractor::extractSymbol(params[i], ctx));

        subs[param->type()] = findSubstitution(env, _args[i])->applyEnv(env, ctx);
    }

    return findSubstitution(subs, ast::ASTTypeCreator::createType(ctr->getTypeConstructor()->getBody(), ctx, subs))->applyEnv(subs, ctx);
}

// TYPED

Typed::Typed() : _type(Type::NotYetDefined()) {

}

Typed::~Typed() {

}

void Typed::setType(Type* type) {
    _type = type;
}

Type* Typed::type() const {
    return _type;
}

}

}
