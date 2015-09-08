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

    virtual Type* substitute(const SubstitutionTable& table, CompCtx_Ptr& ctx) const override {
        return (Type*)this;
    }

    virtual TYPE_KIND getTypeKind() const {
        return TYPE_NYD;
    }

    virtual bool isSubTypeOf(const Type*) const {
        return false;
    }

    virtual Type* apply(const SubstitutionTable&, CompCtx_Ptr&) const {
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

Type* Type::apply(CompCtx_Ptr&) const {
    return const_cast<Type*>(this);
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

Type* Type::findSubstitution(const SubstitutionTable& table, Type* toFind, bool* matched) {
    auto found = table.find(toFind);
    bool m = (found != table.end());

    if (matched) {
        *matched = m;
    }

    return m ? found->second : toFind;
}

bool Type::applyEnvHelper(const SubstitutionTable& env, SubstitutionTable& to) {
    bool matched = false;
    for (auto& pair : to) {
        bool tmp;
        pair.second = findSubstitution(env, pair.second, &tmp);
        matched |= tmp;
    }
    return matched;
}

// PROPER TYPE

ProperType::ProperType(ast::ClassDecl* clss, const SubstitutionTable& substitutionTable)
    : Type(substitutionTable), _class(clss) {

}

ProperType::~ProperType() {

}

Type* ProperType::substitute(const SubstitutionTable& table, CompCtx_Ptr& ctx) const {
    SubstitutionTable copy = _subTable;
    applyEnvHelper(table, copy);
    return ctx->memoryManager().New<ProperType>(_class, copy);
}

TYPE_KIND ProperType::getTypeKind() const { return TYPE_PROPER; }

bool ProperType::isSubTypeOf(const Type* other) const {
    if (ProperType* objother = getIf<ProperType>(other)) {
        if (_class->CanSubtypeClasses::extends(objother->_class)) { // TODO : change that when inheritance is supported.
            const SubstitutionTable& osub = objother->getSubstitutionTable();
            for (const auto& pair : _subTable) {
                const auto& subpair = osub.find(pair.first);
                if (subpair != osub.end() && !pair.second->isSubTypeOf(subpair->second)) { // TODO support contravariance maybe?
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
            if (!oArgTypes[i]->isSubTypeOf(_argTypes[i])) {
                return false;
            }
        }

        return _retType->isSubTypeOf(oRetType);
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

Type* FunctionType::substitute(const SubstitutionTable& table, CompCtx_Ptr& ctx) const {
    SubstitutionTable copy = _subTable;
    applyEnvHelper(table, copy);
    return ctx->memoryManager().New<FunctionType>(_argTypes, _retType, _class, copy);
}

Type* FunctionType::apply(CompCtx_Ptr& ctx) const {
    std::vector<Type*> newArgTypes;
    Type* newRetType;

    for (Type* t : _argTypes) {
        newArgTypes.push_back(findSubstitution(_subTable, t)->apply(ctx));
    }

    newRetType = findSubstitution(_subTable, _retType)->apply(ctx);

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

Type* MethodType::substitute(const SubstitutionTable& table, CompCtx_Ptr& ctx) const {
    SubstitutionTable copy = _subTable;
    applyEnvHelper(table, copy);
    return ctx->memoryManager().New<MethodType>(_owner, _argTypes, _retType, copy);
}

TYPE_KIND MethodType::getTypeKind() const {
    return TYPE_METHOD;
}

bool MethodType::isSubTypeOf(const Type* other) const {
    if (MethodType* m = getIf<MethodType>(other)) {
        const std::vector<Type*>& oArgTypes = m->getArgTypes();
        const Type* oRetType = m->getRetType();

        if (_argTypes.size() != oArgTypes.size()) {
            return false;
        }

        for (size_t i = 0; i < _argTypes.size(); ++i) {
            if (!oArgTypes[i]->isSubTypeOf(_argTypes[i])) {
                return false;
            }
        }

        return _retType->isSubTypeOf(oRetType);
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

Type* MethodType::apply(CompCtx_Ptr& ctx) const {
    std::vector<Type*> newArgTypes;
    Type* newRetType;

    for (Type* t : _argTypes) {
        newArgTypes.push_back(findSubstitution(_subTable, t)->apply(ctx));
    }

    newRetType = findSubstitution(_subTable, _retType)->apply(ctx);

    return ctx->memoryManager().New<MethodType>(_owner, newArgTypes, newRetType, _subTable);
}

ast::ClassDecl* MethodType::getOwner() const {
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

Type* TypeConstructorType::substitute(const SubstitutionTable& table, CompCtx_Ptr& ctx) const {
    SubstitutionTable copy = _subTable;
    applyEnvHelper(table, copy);
    return ctx->memoryManager().New<TypeConstructorType>(_typeConstructor, copy);
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

Type* ConstructorApplyType::substitute(const SubstitutionTable& table, CompCtx_Ptr& ctx) const {
    SubstitutionTable copy = _subTable;
    applyEnvHelper(table, copy);

    std::vector<Type*> substitued(_args.size());
    for (size_t i = 0; i < _args.size(); ++i) {
        substitued[i] = findSubstitution(table, _args[i])->substitute(table, ctx);
    }

    return ctx->memoryManager().New<ConstructorApplyType>(findSubstitution(table, _callee)->substitute(table, ctx), substitued, _pos, copy);
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
    return toRet + "]" + Type::toString();
}

Type* ConstructorApplyType::apply(CompCtx_Ptr& ctx) const {
    TypeConstructorType* ctr = static_cast<TypeConstructorType*>(findSubstitution(_subTable, _callee)->apply(ctx));
    return ast::ASTTypeCreator::createType(ctr->getTypeConstructor()->getBody(), ctx, _args)->apply(ctx);
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
