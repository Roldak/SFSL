//
//  Types.h
//  SFSL
//
//  Created by Romain Beguet on 24.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Types__
#define __SFSL__Types__

#include <iostream>
#include <vector>
#include <map>
#include "Environment.h"
#include "../../Common/MemoryManageable.h"
#include "../../Common/CompilationContext.h"
#include "../../Common/Positionnable.h"

namespace sfsl {

namespace ast {
class TypeExpression;
class ClassDecl;
class TypeConstructorCreation;
}

namespace type {

enum TYPE_KIND { TYPE_NYD, TYPE_TBI, TYPE_PROPER, TYPE_FUNCTION, TYPE_METHOD, TYPE_CONSTRUCTOR_TYPE, TYPE_CONSTRUCTOR_APPLY };

class Typed;

class Type : public common::MemoryManageable {
public:
    Type(const Environment& substitutionTable = {});

    virtual ~Type();

    virtual TYPE_KIND getTypeKind() const = 0;
    virtual bool isSubTypeOf(const Type* other) const = 0;
    virtual bool equals(const Type* other) const;
    virtual std::string toString() const = 0;

    Type* substitute(const Environment& env, CompCtx_Ptr& ctx) const;
    virtual Type* apply(CompCtx_Ptr& ctx) const;
    virtual Type* applyTCCallsOnly(CompCtx_Ptr& ctx) const;

    const Environment& getSubstitutionTable() const;

    static Type* NotYetDefined();

protected:
    friend class ValueConstructorType;

    virtual Type* substituteDeep(const Environment& env, CompCtx_Ptr& ctx) const = 0;

    static Type* DefaultGenericType(ast::TypeExpression* tpe, CompCtx_Ptr& ctx);

    const Environment _subTable;
};

class TypeToBeInferred : public Type {
public:
    TypeToBeInferred(const std::vector<Typed*>& associatedTyped);

    virtual ~TypeToBeInferred();

    virtual TYPE_KIND getTypeKind() const override;
    virtual bool isSubTypeOf(const Type*) const override;
    virtual bool equals(const Type *other) const override;
    virtual std::string toString() const override;

    void assignInferredType(Type* t);

    static TypeToBeInferred* create(const std::vector<Typed*>& toBeInferred, CompCtx_Ptr& ctx);

protected:

    virtual TypeToBeInferred* substituteDeep(const Environment& env, CompCtx_Ptr& ctx) const override;

    std::vector<Typed*> _associatedTyped;
};

class ProperType : public Type {
public:
    ProperType(ast::ClassDecl* clss, const Environment& substitutionTable);

    virtual ~ProperType();

    virtual TYPE_KIND getTypeKind() const override;
    virtual bool isSubTypeOf(const Type* other) const override;
    virtual bool equals(const Type *other) const override;
    virtual std::string toString() const override;

    ast::ClassDecl* getClass() const;

protected:

    virtual ProperType* substituteDeep(const Environment& env, CompCtx_Ptr& ctx) const override;

    ast::ClassDecl* _class;
};

class ValueConstructorType {
public:
    virtual ~ValueConstructorType();

    bool isSubTypeOfValueConstructor(const ValueConstructorType *other) const;
    bool equalsValueConstructor(const ValueConstructorType *other) const;

    const std::vector<ast::TypeExpression*>& getTypeArgs() const;
    const std::vector<Type*>& getArgTypes() const;
    Type* getRetType() const;

    ValueConstructorType* substituteValueConstructor(const Environment& substitutions, CompCtx_Ptr& ctx) const;

    ValueConstructorType* applyValueConstructor(CompCtx_Ptr& ctx) const;

    virtual ValueConstructorType* rebuildValueConstructor(
            const std::vector<ast::TypeExpression*>& typeArgs,
            const std::vector<Type*>& argTypes, Type* retType,
            const Environment& substitutionTable,
            CompCtx_Ptr& ctx) const = 0;

protected:

    ValueConstructorType(const std::vector<ast::TypeExpression*>& typeArgs, const std::vector<Type*>& argTypes, Type* retType);

    virtual const Environment& getValueConstructorSubstitutionTable() const = 0;

    std::vector<ast::TypeExpression*> _typeArgs;
    std::vector<Type*> _argTypes;
    Type* _retType;
};

class FunctionType : public ProperType, public ValueConstructorType {
public:
    FunctionType(const std::vector<ast::TypeExpression*>& typeArgs, const std::vector<Type*>& argTypes, Type* retType, ast::ClassDecl* clss, const Environment& substitutionTable);

    virtual ~FunctionType();

    virtual TYPE_KIND getTypeKind() const override;
    virtual bool isSubTypeOf(const Type* other) const override;
    virtual bool equals(const Type *other) const override;
    virtual std::string toString() const override;

    virtual FunctionType* apply(CompCtx_Ptr& ctx) const override;

    virtual FunctionType* rebuildValueConstructor(
            const std::vector<ast::TypeExpression*>& typeArgs,
            const std::vector<Type*>& argTypes, Type* retType,
            const Environment& substitutionTable,
            CompCtx_Ptr& ctx) const override;

protected:

    virtual FunctionType* substituteDeep(const Environment& env, CompCtx_Ptr& ctx) const override;
    virtual const Environment& getValueConstructorSubstitutionTable() const override;
};

class MethodType : public Type, public ValueConstructorType {
public:
    MethodType(ast::ClassDecl* owner, const std::vector<ast::TypeExpression*>& typeArgs, const std::vector<Type*>& argTypes, Type* retType, const Environment& substitutionTable);

    virtual ~MethodType();

    virtual TYPE_KIND getTypeKind() const override;
    virtual bool isSubTypeOf(const Type *other) const override;
    virtual bool equals(const Type *other) const override;
    virtual std::string toString() const override;

    virtual MethodType* apply(CompCtx_Ptr &ctx) const override;

    virtual MethodType* rebuildValueConstructor(
            const std::vector<ast::TypeExpression*>& typeArgs,
            const std::vector<Type*>& argTypes, Type* retType,
            const Environment& substitutionTable,
            CompCtx_Ptr& ctx) const override;

    ast::ClassDecl* getOwner() const;

    static MethodType* fromFunctionType(const FunctionType* ft, ast::ClassDecl* owner, CompCtx_Ptr& ctx);

protected:

    virtual MethodType* substituteDeep(const Environment& env, CompCtx_Ptr& ctx) const override;
    virtual const Environment& getValueConstructorSubstitutionTable() const override;

private:

    ast::ClassDecl* _owner;
};

class TypeConstructorType : public Type {
public:
    TypeConstructorType(ast::TypeConstructorCreation* typeConstructor, const Environment& substitutionTable);

    virtual ~TypeConstructorType();

    virtual TYPE_KIND getTypeKind() const override;
    virtual bool isSubTypeOf(const Type* other) const override;
    virtual bool equals(const Type *other) const override;
    virtual std::string toString() const override;

    ast::TypeConstructorCreation* getTypeConstructor() const;

protected:

    virtual TypeConstructorType* substituteDeep(const Environment& env, CompCtx_Ptr& ctx) const override;

    ast::TypeConstructorCreation* _typeConstructor;
};

class ConstructorApplyType : public Type {
public:
    ConstructorApplyType(Type* callee, const std::vector<Type*>& args);

    virtual ~ConstructorApplyType();

    virtual TYPE_KIND getTypeKind() const override;
    virtual bool isSubTypeOf(const Type* other) const override;
    virtual std::string toString() const override;

    virtual Type* apply(CompCtx_Ptr& ctx) const override;
    virtual Type* applyTCCallsOnly(CompCtx_Ptr& ctx) const override;

protected:

    virtual ConstructorApplyType* substituteDeep(const Environment& env, CompCtx_Ptr& ctx) const override;

    Type* _callee;
    const std::vector<Type*> _args;
};

/**
 * @brief Interface that represents a node that has a type,
 * like an expression, or some of the symbols.
 */
class Typed {
public:
    Typed();

    virtual ~Typed();

    /**
     * @param type The type to set
     */
    void setType(Type* type);

    /**
     * @return The type of the object
     */
    Type* type() const;

protected:

    Type* _type;
};

template<typename T>
inline T* getIf(const Type* t) {
    return nullptr;
}

template<>
inline TypeToBeInferred* getIf(const Type* t) {
    return (t->getTypeKind() == TYPE_TBI) ? (TypeToBeInferred*)t : nullptr;
}

template<>
inline ProperType* getIf(const Type* t) {
    return (t->getTypeKind() == TYPE_PROPER || t->getTypeKind() == TYPE_FUNCTION) ? (ProperType*)t : nullptr;
}

template<>
inline FunctionType* getIf(const Type* t) {
    return t->getTypeKind() == TYPE_FUNCTION ? (FunctionType*)t : nullptr;
}

template<>
inline MethodType* getIf(const Type* t) {
    return t->getTypeKind() == TYPE_METHOD ? (MethodType*)t : nullptr;
}

template<>
inline ValueConstructorType* getIf(const Type* t) {
    return t->getTypeKind() == TYPE_FUNCTION || t->getTypeKind() == TYPE_METHOD ? (ValueConstructorType*)t : nullptr;
}

template<>
inline TypeConstructorType* getIf(const Type* t) {
    return t->getTypeKind() == TYPE_CONSTRUCTOR_TYPE ? (TypeConstructorType*)t : nullptr;
}

template<>
inline ConstructorApplyType* getIf(const Type* t) {
    return t->getTypeKind() == TYPE_CONSTRUCTOR_APPLY ? (ConstructorApplyType*)t : nullptr;
}

}

}

#endif
