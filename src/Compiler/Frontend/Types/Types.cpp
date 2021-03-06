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

bool envsEqual(const Environment& env1, const Environment& env2, CompCtx_Ptr& ctx) {
    for (Environment::const_iterator env1It = env1.begin(), env2It = env2.begin(), env1End = env1.end();
         env1It != env1End; ++env1It, ++env2It) {

        if (!env1It->value->apply(ctx)->equals(env2It->value->apply(ctx), ctx)) {
            return false;
        }
    }
    return true;
}

// TYPE NOT YET DEFINED

class TypeNotYetDefined : public Type {
public:

    TypeNotYetDefined() {}

    virtual ~TypeNotYetDefined() {}

    virtual TYPE_KIND getTypeKind() const override {
        return TYPE_NYD;
    }

    virtual bool isSubTypeOf(const Type*, CompCtx_Ptr&) const override {
        return false;
    }

    virtual Type* substituteDeep(const Environment&, CompCtx_Ptr&) const override {
        return (Type*)this;
    }

    virtual std::string toString(CompCtx_Ptr*) const override {
        return "<not yet defined>";
    }
};

// TYPE

Type::Type(const Environment &substitutionTable) : _env(substitutionTable) {

}

Type::~Type() {

}

bool Type::equals(const Type* other, CompCtx_Ptr& ctx) const {
    return isSubTypeOf(other, ctx) && other->isSubTypeOf(this, ctx);
}

std::string Type::toString(CompCtx_Ptr* shouldApply) const {
    std::string toRet;
    if (!_env.empty()) {
        toRet += "{";
        for (const auto& pair : _env) {
            toRet += pair.key->toString() + "=>" + (shouldApply ? pair.value->apply(*shouldApply) : pair.value)->toString(shouldApply) + ", ";
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

const Environment& Type::getEnvironment() const {
    return _env;
}

Type* Type::NotYetDefined() {
    static TypeNotYetDefined nyd;
    return &nyd; // all we want is a unique memory area
}

struct DefaultGenericTypeHolder : public common::MemoryManageable {
    virtual ~DefaultGenericTypeHolder() { }

    bool findCachedDefaultGeneric(CompCtx_Ptr& ctx, kind::Kind* ofkind, type::Type** out) const {
        for (const std::pair<kind::Kind*, type::Type*>& defGen : _cachedDefaultGenerics) {
            if (defGen.first->isSubKindOf(ofkind, ctx, false)) {
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
    DefaultGenericTypeHolder* holder = ctx->retrieveContextUserData<DefaultGenericTypeHolder>(DEFAULT_GENERIC_TYPE_HOLDER_ENTRY_NAME);

    // check if already exists (not an optimization, but a needed operation)

    kind::Kind* tpeKind = tpe->kind();
    type::Type* cachedType;

    if (holder->findCachedDefaultGeneric(ctx, tpeKind, &cachedType)) {
        return cachedType;
    }

    // if not, add it

    type::Type* res = nullptr;

    if (ast::TypeParameter* tparam = ast::getIfNodeOfType<ast::TypeParameter>(tpe, ctx)) {
        res = ast::ASTTypeCreator::createType(tparam->getKindNode()->getDefaultType(), ctx);
        holder->cacheDefaultGeneric(tpeKind, res);
    } else {
        ctx->reporter().fatal(*tpe, "Is supposed to be a type parameter");
    }

    return res;
}

// TYPE MUST BE INFERRED

TypeToBeInferred::TypeToBeInferred(const std::vector<Typed*>& associatedTyped) : _associatedTyped(associatedTyped) {

}

TypeToBeInferred::~TypeToBeInferred() {}

TYPE_KIND TypeToBeInferred::getTypeKind() const {
    return TYPE_TBI;
}

bool TypeToBeInferred::isSubTypeOf(const Type*, CompCtx_Ptr&) const {
    return false;
}

bool TypeToBeInferred::equals(const Type* other, CompCtx_Ptr&) const {
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

std::string TypeToBeInferred::toString(CompCtx_Ptr*) const {
    return "<to be inferred>";
}

// PROPER TYPE

ProperType::ProperType(ast::ClassDecl* clss, const Environment& substitutionTable)
    : Type(substitutionTable), _class(clss) {

}

ProperType::~ProperType() {

}

TYPE_KIND ProperType::getTypeKind() const { return TYPE_PROPER; }

bool ProperType::isSubTypeOf(const Type* other, CompCtx_Ptr& ctx) const {
    if (ProperType* objother = getIf<ProperType>(other)) {
        const Environment& osubs = objother->getEnvironment();

        for (const Environment& parentEnv : _class->subTypeInstances(objother->_class)) {

            for (Environment::const_iterator otherIt = osubs.begin(), thisIt = parentEnv.begin(), otherEnd = osubs.end();
                 otherIt != otherEnd; ++otherIt, ++thisIt) {

                Type* val = _env.findSubstOrReturnMe(thisIt->value);

                switch (otherIt->varianceType) {
                case common::VAR_T_IN:
                    if (!otherIt->value->apply(ctx)->isSubTypeOf(val->apply(ctx), ctx))
                        return false;
                    break;
                case common::VAR_T_OUT:
                    if (!val->apply(ctx)->isSubTypeOf(otherIt->value->apply(ctx), ctx))
                        return false;
                    break;
                case common::VAR_T_NONE:
                    if (!val->apply(ctx)->equals(otherIt->value->apply(ctx), ctx))
                        return false;
                    break;
                }
            }
            return true;
        }
    }
    return false;
}

bool ProperType::equals(const Type* other, CompCtx_Ptr& ctx) const {
    if (ProperType* objother = getIf<ProperType>(other)) {
        if (_class == objother->getClass()) {
            return envsEqual(_env, objother->getEnvironment(), ctx);
        }
    }
    return false;
}

std::string ProperType::toString(CompCtx_Ptr* shouldApply) const {
    return _class->getName() + Type::toString(shouldApply);
}

ProperType* ProperType::substituteDeep(const Environment& env, CompCtx_Ptr& ctx) const {
    Environment copy = _env;
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
    : _typeArgs(typeArgs), _argTypes(argTypes), _retType(retType), _cachedApplied(nullptr) {

}

ValueConstructorType::~ValueConstructorType() {

}

bool ValueConstructorType::isSubTypeOfValueConstructor(const ValueConstructorType* other, CompCtx_Ptr& ctx) const {
    const std::vector<ast::TypeExpression*>& oTypeArgs = other->getTypeArgs();
    const std::vector<Type*>& oArgTypes = other->getArgTypes();
    const Type* oRetType = other->getRetType();

    if (_typeArgs.size() != oTypeArgs.size() || _argTypes.size() != oArgTypes.size()) {
        return false;
    }

    for (size_t i = 0; i < _typeArgs.size(); ++i) {
        if (!oTypeArgs[i]->kind()->substitute(other->getValueConstructorEnvironment(), ctx)->apply(ctx)
                ->isSubKindOf(_typeArgs[i]->kind()->substitute(getValueConstructorEnvironment(), ctx)->apply(ctx), ctx, true)) {
            return false;
        }
    }

    for (size_t i = 0; i < _argTypes.size(); ++i) {
        if (!oArgTypes[i]->isSubTypeOf(_argTypes[i], ctx)) {
            return false;
        }
    }

    return _retType->isSubTypeOf(oRetType, ctx);
}

bool ValueConstructorType::equalsValueConstructor(const ValueConstructorType* other, CompCtx_Ptr& ctx) const {
    const std::vector<ast::TypeExpression*>& oTypeArgs = other->getTypeArgs();
    const std::vector<Type*>& oArgTypes = other->getArgTypes();
    const Type* oRetType = other->getRetType();

    if (_typeArgs.size() != oTypeArgs.size() || _argTypes.size() != oArgTypes.size()) {
        return false;
    }

    for (size_t i = 0; i < _typeArgs.size(); ++i) {
        if (!oTypeArgs[i]->kind()->isSubKindOf(_typeArgs[i]->kind(), ctx, true)) {
            return false;
        }
    }

    for (size_t i = 0; i < _argTypes.size(); ++i) {
        if (!oArgTypes[i]->equals(_argTypes[i], ctx)) {
            return false;
        }
    }

    return _retType->equals(oRetType, ctx);
}

ValueConstructorType* ValueConstructorType::substituteValueConstructor(const Environment& substitutions, CompCtx_Ptr& ctx) const {
    Environment currentEnvCopy(getValueConstructorEnvironment());
    currentEnvCopy.substituteAll(substitutions);

    std::vector<Type*> substitued(_argTypes.size());
    for (size_t i = 0; i < _argTypes.size(); ++i) {
        substitued[i] = _argTypes[i]->substitute(substitutions, ctx);
    }

    return rebuildValueConstructor(_typeArgs, substitued, _retType->substitute(substitutions, ctx), currentEnvCopy, ctx);
}

ValueConstructorType* ValueConstructorType::applyValueConstructor(CompCtx_Ptr& ctx) const {
    if (_cachedApplied) {
        return _cachedApplied;
    }

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

    return _cachedApplied = rebuildValueConstructor(self->_typeArgs, applied, self->_retType->apply(ctx), self->getValueConstructorEnvironment(), ctx);
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

bool FunctionType::isSubTypeOf(const Type* other, CompCtx_Ptr& ctx) const {
    if (FunctionType* f = getIf<FunctionType>(other)) {
        return isSubTypeOfValueConstructor(f, ctx);
    }

    return false;
}

bool FunctionType::equals(const Type* other, CompCtx_Ptr& ctx) const {
    if (FunctionType* f = getIf<FunctionType>(other)) {
        return equalsValueConstructor(f, ctx);
    }

    return false;
}

std::string FunctionType::toString(CompCtx_Ptr* shouldApply) const {
    std::string toRet;
    if (_typeArgs.size() > 0) {
        toRet += "[";
        for (size_t i = 0; i < _typeArgs.size() - 1; ++i) {
            toRet += _typeArgs[i]->kind()->toString(true, shouldApply) + ", ";
        }
        toRet += _typeArgs.back()->kind()->toString(true, shouldApply) + "]";
    }

    toRet += "(";

    if (_argTypes.size() > 0) {
        for (size_t i = 0; i < _argTypes.size() - 1; ++i) {
            toRet += _argTypes[i]->toString(shouldApply) + ", ";
        }
        toRet += _argTypes.back()->toString(shouldApply);
    }

    return toRet + ")->" + _retType->toString(shouldApply);
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

const Environment& FunctionType::getValueConstructorEnvironment() const {
    return _env;
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

bool MethodType::isSubTypeOf(const Type* other, CompCtx_Ptr& ctx) const {
    if (MethodType* m = getIf<MethodType>(other)) {
        return isSubTypeOfValueConstructor(m, ctx);
    }

    return false;
}

bool MethodType::equals(const Type* other, CompCtx_Ptr& ctx) const {
    if (MethodType* m = getIf<MethodType>(other)) {
        return equalsValueConstructor(m, ctx);
    }

    return false;
}

std::string MethodType::toString(CompCtx_Ptr* shouldApply) const {
    std::string toRet;
    if (_typeArgs.size() > 0) {
        toRet += "[";
        for (size_t i = 0; i < _typeArgs.size() - 1; ++i) {
            toRet += _typeArgs[i]->kind()->toString(true, shouldApply) + ", ";
        }
        toRet += _typeArgs.back()->kind()->toString(true, shouldApply) + "]";
    }

    toRet += "([" + _owner->getName() + "]";

    for (size_t i = 0; i < _argTypes.size(); ++i) {
        toRet += ", " + _argTypes[i]->toString(shouldApply);
    }

    return toRet + ")->" + _retType->toString(shouldApply);
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
    return ctx->memoryManager().New<MethodType>(owner, ft->getTypeArgs(), ft->getArgTypes(), ft->getRetType(), ft->getEnvironment());
}

const Environment& MethodType::getValueConstructorEnvironment() const {
    return _env;
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

bool TypeConstructorType::isSubTypeOf(const Type* other, CompCtx_Ptr& ctx) const {
    return equals(other, ctx);
}

bool TypeConstructorType::equals(const Type* other, CompCtx_Ptr& ctx) const {
    if (TypeConstructorType* tc = getIf<TypeConstructorType>(other)) {
        return _typeConstructor == tc->getTypeConstructor()
            && envsEqual(_env, tc->getEnvironment(), ctx);
    }
    return false;
}

std::string TypeConstructorType::toString(CompCtx_Ptr* shouldApply) const {
    return _typeConstructor->getName() + Type::toString(shouldApply);
}

TypeConstructorType* TypeConstructorType::substituteDeep(const Environment& env, CompCtx_Ptr& ctx) const {
    Environment copy = _env;
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
    : Type(type::Environment::Empty), _callee(callee), _args(args), _cachedApplied(nullptr), _cachedTCCallsAppliedOnly(nullptr) {

}

ConstructorApplyType::~ConstructorApplyType() {

}

TYPE_KIND ConstructorApplyType::getTypeKind() const {
    return TYPE_CONSTRUCTOR_APPLY;
}

bool ConstructorApplyType::isSubTypeOf(const Type*, CompCtx_Ptr&) const {
    return false;
}

std::string ConstructorApplyType::toString(CompCtx_Ptr* shouldApply) const {
    std::string toRet = _callee->toString(shouldApply) + "[";
    for (size_t i = 0; i < _args.size(); ++i) {
        toRet += _args[i]->toString(shouldApply);
        if (i != _args.size() - 1) {
            toRet += ", ";
        }
    }
    return toRet + "]" + Type::toString(shouldApply);
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
    if (_cachedApplied) {
        return _cachedApplied;
    }
    TypeConstructorType* ctr = static_cast<TypeConstructorType*>(_callee->apply(ctx));
    return _cachedApplied = ast::ASTTypeCreator::evalTypeConstructor(ctr, _args, ctx)->apply(ctx);
}

Type* ConstructorApplyType::applyTCCallsOnly(CompCtx_Ptr& ctx) const {
    if (_cachedTCCallsAppliedOnly) {
        return _cachedTCCallsAppliedOnly;
    }
    TypeConstructorType* ctr = static_cast<TypeConstructorType*>(_callee->apply(ctx));
    return _cachedTCCallsAppliedOnly = ast::ASTTypeCreator::evalTypeConstructor(ctr, _args, ctx)->applyTCCallsOnly(ctx);
}

Type* ConstructorApplyType::getCallee() const {
    return _callee;
}

const std::vector<Type*>& ConstructorApplyType::getArgs() {
    return _args;
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
