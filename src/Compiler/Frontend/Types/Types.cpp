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

#define DEFAULT_GENERIC_TYPE_HOLDER_ENTRY_NAME "DefaultGenericTypeHolder@sfsl::type::DefaultGenericType"

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

    virtual Type* substituteDeep(const Environment&, CompCtx_Ptr&) const override {
        return (Type*)this;
    }

    virtual std::string toString() const override {
        return "<not yet defined>";
    }
};

// TYPE

Type::Type(const Environment &substitutionTable) : _subTable(substitutionTable) {

}

Type::~Type() {

}

bool Type::equals(const Type* other) const {
    return isSubTypeOf(other) && other->isSubTypeOf(this);
}

std::string Type::toString() const {
    std::string toRet /*= utils::T_toString(this)*/;
    if (!_subTable.empty()) {
        toRet += "{";
        for (const auto& pair : _subTable) {
            toRet += pair.key->toString() + "=>" + pair.value->toString() + ", ";
        }
        toRet = toRet.substr(0, toRet.size() - 2) + "}";
    }
    return toRet;
}


Type* Type::substitute(const Environment& env, CompCtx_Ptr& ctx) const {
    return env.findSubstOrReturnMe(const_cast<Type*>(this))->substituteDeep(env, ctx);
}

Type* Type::apply(CompCtx_Ptr&) const {
    return const_cast<Type*>(this);
}

Type* Type::applyTCCallsOnly(CompCtx_Ptr&) const {
    return const_cast<Type*>(this);
}

const Environment& Type::getSubstitutionTable() const {
    return _subTable;
}

Type* Type::NotYetDefined() {
    static TypeNotYetDefined nyd;
    return &nyd; // all we want is a unique memory area
}

struct DefaultGenericTypeHolder : public common::MemoryManageable {
    virtual ~DefaultGenericTypeHolder() { }

    bool findCachedDefaultGeneric(kind::Kind* ofkind, type::Type** out) const {
        for (const std::pair<kind::Kind*, type::Type*>& defGen : _cachedDefaultGenerics) {
            if (defGen.first->isSubKindOf(ofkind, false)) {
                *out = defGen.second;
                return true;
            }
        }
        return false;
    }

    void cacheDefaultGeneric(kind::Kind* ofkind, type::Type* ofType) {
        _cachedDefaultGenerics.push_back(std::make_pair(ofkind, ofType));
    }

private:

    std::vector<std::pair<kind::Kind*, type::Type*>> _cachedDefaultGenerics;
};

Type* Type::DefaultGenericType(ast::TypeExpression* tpe, CompCtx_Ptr& ctx) {
    static ast::ProperTypeKindSpecifier proper;

    DefaultGenericTypeHolder* holder = ctx->retrieveContextUserData<DefaultGenericTypeHolder>(DEFAULT_GENERIC_TYPE_HOLDER_ENTRY_NAME);

    // check if already exists (not an optimization, but a needed operation)

    kind::Kind* tpeKind = tpe->kind();
    type::Type* cachedType;

    if (holder->findCachedDefaultGeneric(tpeKind, &cachedType)) {
        return cachedType;
    }

    // if not, add it

    ast::KindSpecifyingExpression* kse;

    if (ast::isNodeOfType<ast::TypeParameter>(tpe, ctx)) {
        kse = static_cast<ast::TypeParameter*>(tpe)->getKindNode();
    } else {
        kse = &proper;
    }

    type::Type* res = ast::ASTDefaultTypeFromKindCreator::createDefaultTypeFromKind(kse, ctx);

    holder->cacheDefaultGeneric(tpeKind, res);

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

bool TypeToBeInferred::equals(const Type* other) const {
    return this == other;
}

TypeToBeInferred* TypeToBeInferred::substituteDeep(const Environment&, CompCtx_Ptr&) const {
    return (TypeToBeInferred*)this;
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

ProperType::ProperType(ast::ClassDecl* clss, const Environment& substitutionTable)
    : Type(substitutionTable), _class(clss) {

}

ProperType::~ProperType() {

}

TYPE_KIND ProperType::getTypeKind() const { return TYPE_PROPER; }

bool ProperType::isSubTypeOf(const Type* other) const {
    if (ProperType* objother = getIf<ProperType>(other)) {
        if (_class->CanSubtypeClasses::extends(objother->_class)) {
            const Environment& osubs = objother->getSubstitutionTable();

            for (const Environment::Substitution& osub : osubs) {
                const auto& sub = _subTable.find(osub.key);

                if (sub == _subTable.end()) {
                    return false;
                } else switch (osub.varianceType) {
                case common::VAR_T_IN:
                    if (!osub.value->isSubTypeOf(sub->value))
                        return false;
                    break;
                case common::VAR_T_OUT:
                    if (!sub->value->isSubTypeOf(osub.value))
                        return false;
                    break;
                default:
                    if (!sub->value->equals(osub.value))
                        return false;
                    break;
                }
            }
            return true;
        }
    }
    return false;
}

bool ProperType::equals(const Type* other) const {
    if (ProperType* objother = getIf<ProperType>(other)) {
        if (_class == objother->getClass()) {
            return _subTable.equals(objother->getSubstitutionTable());
        }
    }
    return false;
}

std::string ProperType::toString() const {
    return _class->getName() + Type::toString();
}

ProperType* ProperType::substituteDeep(const Environment& env, CompCtx_Ptr& ctx) const {
    Environment copy = _subTable;
    if (copy.substituteAll(env)) {
        return ctx->memoryManager().New<ProperType>(_class, copy);
    } else {
        return const_cast<ProperType*>(this);
    }
}

ast::ClassDecl* ProperType::getClass() const {
    return _class;
}

// VALUE CONSTRUCTOR TYPE

ValueConstructorType::ValueConstructorType(const std::vector<ast::TypeExpression*>& typeArgs,
                                           const std::vector<Type*>& argTypes, Type* retType)
    : _typeArgs(typeArgs), _argTypes(argTypes), _retType(retType) {

}

ValueConstructorType::~ValueConstructorType() {

}

bool ValueConstructorType::isSubTypeOfValueConstructor(const ValueConstructorType* other) const {
    const std::vector<ast::TypeExpression*>& oTypeArgs = other->getTypeArgs();
    const std::vector<Type*>& oArgTypes = other->getArgTypes();
    const Type* oRetType = other->getRetType();

    if (_typeArgs.size() != oTypeArgs.size() || _argTypes.size() != oArgTypes.size()) {
        return false;
    }

    for (size_t i = 0; i < _typeArgs.size(); ++i) {
        if (!oTypeArgs[i]->kind()->isSubKindOf(_typeArgs[i]->kind(), true)) {
            return false;
        }
    }

    for (size_t i = 0; i < _argTypes.size(); ++i) {
        if (!oArgTypes[i]->isSubTypeOf(_argTypes[i])) {
            return false;
        }
    }

    return _retType->isSubTypeOf(oRetType);
}

bool ValueConstructorType::equalsValueConstructor(const ValueConstructorType* other) const {
    const std::vector<ast::TypeExpression*>& oTypeArgs = other->getTypeArgs();
    const std::vector<Type*>& oArgTypes = other->getArgTypes();
    const Type* oRetType = other->getRetType();

    if (_typeArgs.size() != oTypeArgs.size() || _argTypes.size() != oArgTypes.size()) {
        return false;
    }

    for (size_t i = 0; i < _typeArgs.size(); ++i) {
        if (!oTypeArgs[i]->kind()->isSubKindOf(_typeArgs[i]->kind(), true)) {
            return false;
        }
    }

    for (size_t i = 0; i < _argTypes.size(); ++i) {
        if (!oArgTypes[i]->equals(_argTypes[i])) {
            return false;
        }
    }

    return _retType->equals(oRetType);
}

ValueConstructorType* ValueConstructorType::substituteValueConstructor(const Environment& substitutions, CompCtx_Ptr& ctx) const {
    Environment currentEnvCopy(getValueConstructorSubstitutionTable());
    currentEnvCopy.substituteAll(substitutions);

    std::vector<Type*> substitued(_argTypes.size());
    for (size_t i = 0; i < _argTypes.size(); ++i) {
        substitued[i] = _argTypes[i]->substitute(substitutions, ctx);
    }

    return rebuildValueConstructor(_typeArgs, substitued, _retType->substitute(substitutions, ctx), currentEnvCopy, ctx);
}

ValueConstructorType* ValueConstructorType::applyValueConstructor(CompCtx_Ptr& ctx) const {
    const ValueConstructorType* self = this;

    if (_typeArgs.size() > 0) {
        std::vector<Type*> defaultTypes(_typeArgs.size());
        for (size_t i = 0; i < defaultTypes.size(); ++i) {
            defaultTypes[i] = Type::DefaultGenericType(_typeArgs[i], ctx);
        }
        Environment sub = ast::ASTTypeCreator::buildEnvironmentFromTypeParameterInstantiation(_typeArgs, defaultTypes, ctx);
        self = self->substituteValueConstructor(sub, ctx);
    }

    std::vector<Type*> applied(self->_argTypes.size());
    for (size_t i = 0; i < self->_argTypes.size(); ++i) {
        applied[i] = self->_argTypes[i]->apply(ctx);
    }

    return rebuildValueConstructor(self->_typeArgs, applied, self->_retType->apply(ctx), self->getValueConstructorSubstitutionTable(), ctx);
}

const std::vector<ast::TypeExpression*>& ValueConstructorType::getTypeArgs() const {
    return _typeArgs;
}

const std::vector<Type*>& ValueConstructorType::getArgTypes() const {
    return _argTypes;
}

Type* ValueConstructorType::getRetType() const {
    return _retType;
}

// FUNCTION TYPE

FunctionType::FunctionType(
        const std::vector<ast::TypeExpression*>& typeArgs, const std::vector<Type*>& argTypes, Type* retType,
        ast::ClassDecl* clss, const Environment& substitutionTable)
    : ProperType(clss, substitutionTable), ValueConstructorType(typeArgs, argTypes, retType) {

}

FunctionType::~FunctionType() {

}

TYPE_KIND FunctionType::getTypeKind() const {
    return TYPE_FUNCTION;
}

bool FunctionType::isSubTypeOf(const Type* other) const {
    if (FunctionType* f = getIf<FunctionType>(other)) {
        return isSubTypeOfValueConstructor(f);
    }

    return false;
}

bool FunctionType::equals(const Type* other) const {
    if (FunctionType* f = getIf<FunctionType>(other)) {
        return equalsValueConstructor(f);
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

FunctionType* FunctionType::substituteDeep(const Environment& env, CompCtx_Ptr& ctx) const {
    return static_cast<FunctionType*>(substituteValueConstructor(env, ctx));
}

FunctionType* FunctionType::apply(CompCtx_Ptr& ctx) const {
    return static_cast<FunctionType*>(applyValueConstructor(ctx));
}

FunctionType* FunctionType::rebuildValueConstructor(
        const std::vector<ast::TypeExpression*>& typeArgs,
        const std::vector<Type*>& argTypes, Type* retType,
        const Environment& substitutionTable,
        CompCtx_Ptr& ctx) const {
    return ctx->memoryManager().New<FunctionType>(typeArgs, argTypes, retType, _class, substitutionTable);
}

const Environment& FunctionType::getValueConstructorSubstitutionTable() const {
    return _subTable;
}

// METHOD TYPE

MethodType::MethodType(ast::ClassDecl* owner,
                       const std::vector<ast::TypeExpression*>& typeArgs, const std::vector<Type*>& argTypes, Type* retType,
                       const Environment& substitutionTable)
    : Type(substitutionTable), ValueConstructorType(typeArgs, argTypes, retType), _owner(owner) {

}

MethodType::~MethodType() {

}

TYPE_KIND MethodType::getTypeKind() const {
    return TYPE_METHOD;
}

bool MethodType::isSubTypeOf(const Type* other) const {
    if (MethodType* m = getIf<MethodType>(other)) {
        return isSubTypeOfValueConstructor(m);
    }

    return false;
}

bool MethodType::equals(const Type* other) const {
    if (MethodType* m = getIf<MethodType>(other)) {
        return equalsValueConstructor(m);
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

MethodType* MethodType::substituteDeep(const Environment& env, CompCtx_Ptr& ctx) const {
    return static_cast<MethodType*>(substituteValueConstructor(env, ctx));
}

MethodType* MethodType::apply(CompCtx_Ptr& ctx) const {
    return static_cast<MethodType*>(applyValueConstructor(ctx));
}

MethodType* MethodType::rebuildValueConstructor(
        const std::vector<ast::TypeExpression*>& typeArgs,
        const std::vector<Type*>& argTypes, Type* retType,
        const Environment& substitutionTable,
        CompCtx_Ptr& ctx) const {
    return ctx->memoryManager().New<MethodType>(_owner, typeArgs, argTypes, retType, substitutionTable);
}

ast::ClassDecl* MethodType::getOwner() const {
    return _owner;
}

MethodType* MethodType::fromFunctionType(const FunctionType* ft, ast::ClassDecl* owner, CompCtx_Ptr& ctx) {
    return ctx->memoryManager().New<MethodType>(owner, ft->getTypeArgs(), ft->getArgTypes(), ft->getRetType(), ft->getSubstitutionTable());
}

const Environment& MethodType::getValueConstructorSubstitutionTable() const {
    return _subTable;
}

// TYPE CONSTRUCTOR

TypeConstructorType::TypeConstructorType(ast::TypeConstructorCreation*typeConstructor, const Environment& substitutionTable)
    : Type(substitutionTable), _typeConstructor(typeConstructor) {

}

TypeConstructorType::~TypeConstructorType() {

}

TYPE_KIND TypeConstructorType::getTypeKind() const {
    return TYPE_CONSTRUCTOR_TYPE;
}

bool TypeConstructorType::isSubTypeOf(const Type* other) const {
    return equals(other);
}

bool TypeConstructorType::equals(const Type* other) const {
    if (TypeConstructorType* tc = getIf<TypeConstructorType>(other)) {
        return _typeConstructor == tc->getTypeConstructor()
            && _subTable.equals(other->getSubstitutionTable());
    }
    return false;
}

std::string TypeConstructorType::toString() const {
    return _typeConstructor->getName() + Type::toString();
}

TypeConstructorType* TypeConstructorType::substituteDeep(const Environment& env, CompCtx_Ptr& ctx) const {
    Environment copy = _subTable;
    if (copy.substituteAll(env)) {
        return ctx->memoryManager().New<TypeConstructorType>(_typeConstructor, copy);
    } else {
        return const_cast<TypeConstructorType*>(this);
    }
}

ast::TypeConstructorCreation* TypeConstructorType::getTypeConstructor() const {
    return _typeConstructor;
}

// CONSTRUCTOR APPLY TYPE

ConstructorApplyType::ConstructorApplyType(Type* callee, const std::vector<Type*>& args)
    : Type(type::Environment::Empty), _callee(callee), _args(args) {

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

ConstructorApplyType* ConstructorApplyType::substituteDeep(const Environment& env, CompCtx_Ptr& ctx) const {
    if (env.empty()) {
        return const_cast<ConstructorApplyType*>(this);
    }

    std::vector<Type*> substitued(_args.size());
    for (size_t i = 0; i < _args.size(); ++i) {
        substitued[i] = _args[i]->substitute(env, ctx);
    }

    return ctx->memoryManager().New<ConstructorApplyType>(_callee->substitute(env, ctx), substitued);
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
