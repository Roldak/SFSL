//
//  Types.cpp
//  SFSL
//
//  Created by Romain Beguet on 24.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Types.h"
#include "../AST/Nodes/TypeExpressions.h"
#include "../AST/Nodes/KindExpressions.h"

#include "../AST/Visitors/ASTTypeCreator.h"
#include "../AST/Visitors/ASTTypeIdentifier.h"
#include "../AST/Visitors/ASTSymbolExtractor.h"
#include "../AST/Visitors/ASTKindCreator.h"

namespace sfsl {

namespace type {

// TYPE NOT YET DEFINED

class TypeNotYetDefined : public Type {
public:

    TypeNotYetDefined() {}

    virtual ~TypeNotYetDefined() {}

    virtual TYPE_KIND getTypeKind() const override {
        return TYPE_NYD;
    }

    virtual bool isSubTypeOf(const Type*) const override {
        return false;
    }

    virtual Type* substitute(const SubstitutionTable&, CompCtx_Ptr&) const override {
        return (Type*)this;
    }

    virtual std::string toString() const override {
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

Type* Type::applyTCCallsOnly(CompCtx_Ptr&) const {
    return const_cast<Type*>(this);
}

std::string Type::toString() const {
    std::string toRet /*= utils::T_toString(this)*/;
    if (!_subTable.empty()) {
        toRet += "{";
        for (const auto& pair : _subTable) {
            toRet += pair.first->toString() + "=>" + pair.second->toString() + ", ";
        }
        toRet = toRet.substr(0, toRet.size() - 2) + "}";
    }
    return toRet;
}

const SubstitutionTable& Type::getSubstitutionTable() const {
    return _subTable;
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

Type* Type::NotYetDefined() {
    static TypeNotYetDefined nyd;
    return &nyd; // all we want is a unique memory area
}

std::string Type::debugSubstitutionTableToString(const SubstitutionTable& table) {
    return std::accumulate(table.begin(), table.end(), std::string("{"), [](const std::string& str, const std::pair<Type*, Type*>& pair) {
        return str + pair.first->toString() + " => " + pair.second->toString() + " ; ";
    }) + "}";
}

Type* Type::DefaultGenericType(ast::TypeExpression* tpe, CompCtx_Ptr& ctx) {
    static ast::ProperTypeKindSpecifier proper;
    static std::vector<std::pair<kind::Kind*, type::Type*>> defaultGenerics;

    // check if already exists (not an optimization, but a needed operation)

    kind::Kind* tpeKind = tpe->kind();

    for (const std::pair<kind::Kind*, type::Type*>& defGen : defaultGenerics) {
        if (defGen.first->isSubKindOf(tpeKind)) {
            return defGen.second;
        }
    }

    // if not, add it

    ast::KindSpecifyingExpression* kse;

    if (ast::isNodeOfType<ast::KindSpecifier>(tpe, ctx)) {
        kse = static_cast<ast::KindSpecifier*>(tpe)->getKindNode();
    } else {
        kse = &proper;
    }

    type::Type* res = ast::ASTDefaultTypeFromKindCreator::createDefaultTypeFromKind(kse, ctx);
    defaultGenerics.push_back(std::make_pair(tpeKind, res));

    return res;
}

// TYPE MUST BE INFERRED

TypeToBeInferred::TypeToBeInferred(const std::vector<Typed*>& associatedTyped) : _associatedTyped(associatedTyped) {

}

TypeToBeInferred::~TypeToBeInferred() {}

TYPE_KIND TypeToBeInferred::getTypeKind() const {
    return TYPE_TBI;
}

bool TypeToBeInferred::isSubTypeOf(const Type*) const {
    return false;
}

Type* TypeToBeInferred::substitute(const SubstitutionTable&, CompCtx_Ptr&) const {
    return (Type*)this;
}

void TypeToBeInferred::assignInferredType(Type* t) {
    for (Typed* tped : _associatedTyped) {
        tped->setType(t);
    }
}

TypeToBeInferred* TypeToBeInferred::create(const std::vector<Typed*>& toBeInferred, CompCtx_Ptr& ctx) {
    return ctx->memoryManager().New<TypeToBeInferred>(toBeInferred);
}

std::string TypeToBeInferred::toString() const {
    return "<to be inferred>";
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
        if (_class->CanSubtypeClasses::extends(objother->_class)) {
            const SubstitutionTable& osub = objother->getSubstitutionTable();

            //std::cout << toString() << " : " << debugSubstitutionTableToString(getSubstitutionTable()) << std::endl;
            //std::cout << other->toString() << " : " << debugSubstitutionTableToString(other->getSubstitutionTable()) << std::endl;

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

ProperType* ProperType::substitute(const SubstitutionTable& table, CompCtx_Ptr& ctx) const {
    SubstitutionTable copy = _subTable;
    if (applyEnvHelper(table, copy)) {
        return ctx->memoryManager().New<ProperType>(_class, copy);
    } else {
        return const_cast<ProperType*>(this);
    }
}

ast::ClassDecl* ProperType::getClass() const {
    return _class;
}

// FUNCTION TYPE

FunctionType::FunctionType(
        const std::vector<ast::TypeExpression*>& typeArgs, const std::vector<Type*>& argTypes, Type* retType,
        ast::ClassDecl* clss, const SubstitutionTable& substitutionTable)
    : ProperType(clss, substitutionTable), _typeArgs(typeArgs), _argTypes(argTypes), _retType(retType) {

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

FunctionType* FunctionType::substitute(const SubstitutionTable& table, CompCtx_Ptr& ctx) const {
    SubstitutionTable copy = _subTable;
    applyEnvHelper(table, copy);

    std::vector<Type*> substitued(_argTypes.size());
    for (size_t i = 0; i < _argTypes.size(); ++i) {
        substitued[i] = findSubstitution(table, _argTypes[i])->substitute(table, ctx);
    }

    return ctx->memoryManager().New<FunctionType>(_typeArgs, substitued, findSubstitution(table, _retType)->substitute(table, ctx), _class, copy);
}

FunctionType* FunctionType::apply(CompCtx_Ptr& ctx) const {
    const FunctionType* self = this;

    if (_typeArgs.size() > 0) {
        std::vector<Type*> defaultTypes(_typeArgs.size());
        for (size_t i = 0; i < defaultTypes.size(); ++i) {
            defaultTypes[i] = Type::DefaultGenericType(_typeArgs[i], ctx);
        }
        SubstitutionTable sub = ast::ASTTypeCreator::buildSubstitutionTableFromTypeParameterInstantiation(_typeArgs, defaultTypes, ctx);
        self = self->substitute(sub, ctx);
    }

    std::vector<Type*> applied(self->_argTypes.size());
    for (size_t i = 0; i < self->_argTypes.size(); ++i) {
        applied[i] = self->_argTypes[i]->apply(ctx);
    }

    return ctx->memoryManager().New<FunctionType>(std::vector<ast::TypeExpression*>(), applied, self->_retType->apply(ctx), self->_class, self->_subTable);
}

const std::vector<ast::TypeExpression*>& FunctionType::getTypeArgs() const {
    return _typeArgs;
}

const std::vector<Type*>& FunctionType::getArgTypes() const {
    return _argTypes;
}

Type* FunctionType::getRetType() const {
    return _retType;
}

// METHOD TYPE

MethodType::MethodType(ast::ClassDecl* owner,
                       const std::vector<ast::TypeExpression*>& typeArgs, const std::vector<Type*>& argTypes, Type* retType,
                       const SubstitutionTable& substitutionTable)
    : Type(substitutionTable), _owner(owner), _typeArgs(typeArgs), _argTypes(argTypes), _retType(retType) {

}

MethodType::~MethodType() {

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

MethodType* MethodType::substitute(const SubstitutionTable& table, CompCtx_Ptr& ctx) const {
    SubstitutionTable copy = _subTable;
    applyEnvHelper(table, copy);

    std::vector<Type*> substitued(_argTypes.size());
    for (size_t i = 0; i < _argTypes.size(); ++i) {
        substitued[i] = findSubstitution(table, _argTypes[i])->substitute(table, ctx);
    }

    return ctx->memoryManager().New<MethodType>(_owner, _typeArgs, substitued, findSubstitution(table, _retType)->substitute(table, ctx), copy);
}

MethodType* MethodType::apply(CompCtx_Ptr& ctx) const {
    const MethodType* self = this;

    if (_typeArgs.size() > 0) {
        std::vector<Type*> defaultTypes(_typeArgs.size());
        for (size_t i = 0; i < defaultTypes.size(); ++i) {
            defaultTypes[i] = Type::DefaultGenericType(_typeArgs[i], ctx);
        }
        SubstitutionTable sub = ast::ASTTypeCreator::buildSubstitutionTableFromTypeParameterInstantiation(_typeArgs, defaultTypes, ctx);
        self = self->substitute(sub, ctx);
    }

    std::vector<Type*> applied(self->_argTypes.size());
    for (size_t i = 0; i < self->_argTypes.size(); ++i) {
        applied[i] = self->_argTypes[i]->apply(ctx);
    }

    return ctx->memoryManager().New<MethodType>(self->_owner, std::vector<ast::TypeExpression*>(), applied, self->_retType->apply(ctx), self->_subTable);
}

ast::ClassDecl* MethodType::getOwner() const {
    return _owner;
}

const std::vector<ast::TypeExpression*>& MethodType::getTypeArgs() const {
    return _typeArgs;
}

const std::vector<Type*>& MethodType::getArgTypes() const {
    return _argTypes;
}

Type* MethodType::getRetType() const {
    return _retType;
}

MethodType* MethodType::fromFunctionType(const FunctionType* ft, ast::ClassDecl* owner, CompCtx_Ptr& ctx) {
    return ctx->memoryManager().New<MethodType>(owner, ft->getTypeArgs(), ft->getArgTypes(), ft->getRetType(), ft->getSubstitutionTable());
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

TypeConstructorType* TypeConstructorType::substitute(const SubstitutionTable& table, CompCtx_Ptr& ctx) const {
    SubstitutionTable copy = _subTable;
    if (applyEnvHelper(table, copy)) {
        return ctx->memoryManager().New<TypeConstructorType>(_typeConstructor, copy);
    } else {
        return const_cast<TypeConstructorType*>(this);
    }
}

ast::TypeConstructorCreation* TypeConstructorType::getTypeConstructor() const {
    return _typeConstructor;
}

// CONSTRUCTOR APPLY TYPE

ConstructorApplyType::ConstructorApplyType(Type* callee, const std::vector<Type*>& args,
                                           const SubstitutionTable& substitutionTable)
    : Type(substitutionTable), _callee(callee), _args(args) {

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
    return toRet + "]" + Type::toString();
}

Type* ConstructorApplyType::substitute(const SubstitutionTable& table, CompCtx_Ptr& ctx) const {
    if (table.empty()) {
        return const_cast<ConstructorApplyType*>(this);
    }

    SubstitutionTable copy = _subTable;
    applyEnvHelper(table, copy);

    std::vector<Type*> substitued(_args.size());
    for (size_t i = 0; i < _args.size(); ++i) {
        substitued[i] = findSubstitution(table, _args[i])->substitute(table, ctx);
    }

    return ctx->memoryManager().New<ConstructorApplyType>(findSubstitution(table, _callee)->substitute(table, ctx), substitued, copy);
}

Type* ConstructorApplyType::apply(CompCtx_Ptr& ctx) const {
    TypeConstructorType* ctr = static_cast<TypeConstructorType*>(_callee->apply(ctx));
    return ast::ASTTypeCreator::evalTypeConstructor(ctr, _args, ctx)->apply(ctx);
}

Type* ConstructorApplyType::applyTCCallsOnly(CompCtx_Ptr& ctx) const {
    TypeConstructorType* ctr = static_cast<TypeConstructorType*>(_callee->apply(ctx));
    return ast::ASTTypeCreator::evalTypeConstructor(ctr, _args, ctx)->applyTCCallsOnly(ctx);
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
