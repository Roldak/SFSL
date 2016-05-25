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
#include "SubstitutionTable.h"
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
    Type(const SubstitutionTable& substitutionTable = {});

    virtual ~Type();

    virtual TYPE_KIND getTypeKind() const = 0;
    virtual bool isSubTypeOf(const Type* other) const = 0;
    virtual bool equals(const Type* other) const;
    virtual std::string toString() const = 0;

    virtual Type* substitute(const SubstitutionTable& table, CompCtx_Ptr& ctx) const = 0;
    virtual Type* apply(CompCtx_Ptr& ctx) const;
    virtual Type* applyTCCallsOnly(CompCtx_Ptr& ctx) const;

    const SubstitutionTable& getSubstitutionTable() const;

    static Type* findSubstitution(const SubstitutionTable& table, Type* toFind, bool* matched = nullptr);
    static bool applyEnvHelper(const SubstitutionTable& env, SubstitutionTable& to);
    static bool substitutionsEquals(const SubstitutionTable& env1, const SubstitutionTable& env2);

    static Type* NotYetDefined();

    static std::string debugSubstitutionTableToString(const SubstitutionTable& table);

protected:
    friend class ValueConstructorType;

    static Type* DefaultGenericType(ast::TypeExpression* tpe, CompCtx_Ptr& ctx);

    const SubstitutionTable _subTable;
};

class TypeToBeInferred : public Type {
public:
    TypeToBeInferred(const std::vector<Typed*>& associatedTyped);

    virtual ~TypeToBeInferred();

    virtual TYPE_KIND getTypeKind() const override;
    virtual bool isSubTypeOf(const Type*) const override;
    virtual bool equals(const Type *other) const override;
    virtual std::string toString() const override;

    virtual Type* substitute(const SubstitutionTable& table, CompCtx_Ptr& ctx) const override;

    void assignInferredType(Type* t);

    static TypeToBeInferred* create(const std::vector<Typed*>& toBeInferred, CompCtx_Ptr& ctx);

private:

    std::vector<Typed*> _associatedTyped;
};

class ProperType : public Type {
public:
    ProperType(ast::ClassDecl* clss, const SubstitutionTable& substitutionTable);

    virtual ~ProperType();

    virtual TYPE_KIND getTypeKind() const override;
    virtual bool isSubTypeOf(const Type* other) const override;
    virtual bool equals(const Type *other) const override;
    virtual std::string toString() const override;

    virtual ProperType* substitute(const SubstitutionTable& table, CompCtx_Ptr& ctx) const override;

    ast::ClassDecl* getClass() const;

protected:

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

    ValueConstructorType* substituteValueConstructor(const SubstitutionTable& substitutions, CompCtx_Ptr& ctx) const;

    ValueConstructorType* applyValueConstructor(CompCtx_Ptr& ctx) const;

    virtual ValueConstructorType* rebuildValueConstructor(
            const std::vector<ast::TypeExpression*>& typeArgs,
            const std::vector<Type*>& argTypes, Type* retType,
            const SubstitutionTable& substitutionTable,
            CompCtx_Ptr& ctx) const = 0;

protected:
    ValueConstructorType(const std::vector<ast::TypeExpression*>& typeArgs, const std::vector<Type*>& argTypes, Type* retType);

    virtual const SubstitutionTable& getValueConstructorSubstitutionTable() const = 0;

    std::vector<ast::TypeExpression*> _typeArgs;
    std::vector<Type*> _argTypes;
    Type* _retType;
};

class FunctionType : public ProperType, public ValueConstructorType {
public:
    FunctionType(const std::vector<ast::TypeExpression*>& typeArgs, const std::vector<Type*>& argTypes, Type* retType, ast::ClassDecl* clss, const SubstitutionTable& substitutionTable);

    virtual ~FunctionType();

    virtual TYPE_KIND getTypeKind() const override;
    virtual bool isSubTypeOf(const Type* other) const override;
    virtual bool equals(const Type *other) const override;
    virtual std::string toString() const override;

    virtual FunctionType* substitute(const SubstitutionTable& table, CompCtx_Ptr& ctx) const override;
    virtual FunctionType* apply(CompCtx_Ptr& ctx) const override;

    virtual FunctionType* rebuildValueConstructor(
            const std::vector<ast::TypeExpression*>& typeArgs,
            const std::vector<Type*>& argTypes, Type* retType,
            const SubstitutionTable& substitutionTable,
            CompCtx_Ptr& ctx) const override;

protected:
    const SubstitutionTable& getValueConstructorSubstitutionTable() const override;
};

class MethodType : public Type, public ValueConstructorType {
public:
    MethodType(ast::ClassDecl* owner, const std::vector<ast::TypeExpression*>& typeArgs, const std::vector<Type*>& argTypes, Type* retType, const SubstitutionTable& substitutionTable);

    virtual ~MethodType();

    virtual TYPE_KIND getTypeKind() const override;
    virtual bool isSubTypeOf(const Type *other) const override;
    virtual bool equals(const Type *other) const override;
    virtual std::string toString() const override;

    virtual MethodType* substitute(const SubstitutionTable& table, CompCtx_Ptr& ctx) const override;
    virtual MethodType* apply(CompCtx_Ptr &ctx) const override;

    virtual MethodType* rebuildValueConstructor(
            const std::vector<ast::TypeExpression*>& typeArgs,
            const std::vector<Type*>& argTypes, Type* retType,
            const SubstitutionTable& substitutionTable,
            CompCtx_Ptr& ctx) const override;

    ast::ClassDecl* getOwner() const;

    static MethodType* fromFunctionType(const FunctionType* ft, ast::ClassDecl* owner, CompCtx_Ptr& ctx);

protected:
    const SubstitutionTable& getValueConstructorSubstitutionTable() const override;

private:
    ast::ClassDecl* _owner;
};

class TypeConstructorType : public Type {
public:
    TypeConstructorType(ast::TypeConstructorCreation* typeConstructor, const SubstitutionTable& substitutionTable);

    virtual ~TypeConstructorType();

    virtual TYPE_KIND getTypeKind() const override;
    virtual bool isSubTypeOf(const Type* other) const override;
    virtual bool equals(const Type *other) const override;
    virtual std::string toString() const override;

    virtual TypeConstructorType* substitute(const SubstitutionTable& table, CompCtx_Ptr& ctx) const override;

    ast::TypeConstructorCreation* getTypeConstructor() const;

private:

    ast::TypeConstructorCreation* _typeConstructor;
};

class ConstructorApplyType : public Type {
public:
    ConstructorApplyType(Type* callee, const std::vector<Type*>& args);

    virtual ~ConstructorApplyType();

    virtual TYPE_KIND getTypeKind() const override;
    virtual bool isSubTypeOf(const Type* other) const override;
    virtual std::string toString() const override;

    virtual Type* substitute(const SubstitutionTable& table, CompCtx_Ptr& ctx) const override;
    virtual Type* apply(CompCtx_Ptr& ctx) const override;
    virtual Type* applyTCCallsOnly(CompCtx_Ptr& ctx) const override;

private:

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
    if (FunctionType* ft = getIf<FunctionType>(t)) {
        return ft;
    } else if (MethodType* mt = getIf<MethodType>(t)) {
        return mt;
    } else {
        return nullptr;
    }
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
