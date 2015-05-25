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
#include "../Common/MemoryManageable.h"
#include "../Common/CompilationContext.h"
#include "../Common/Positionnable.h"

namespace sfsl {

namespace ast {
    class ClassDecl;
    class TypeConstructorCreation;
}

namespace type {

enum TYPE_KIND { TYPE_NYD, TYPE_PROPER, TYPE_CONSTRUCTOR, TYPE_CONSTRUCTOR_APPLY };

class Type;

typedef std::map<Type*, Type*> SubstitutionTable;

class Type : public common::MemoryManageable {
public:
    Type(const SubstitutionTable& substitutionTable = {});

    virtual ~Type();

    virtual TYPE_KIND getTypeKind() const = 0;
    virtual bool isSubTypeOf(const Type* other) const = 0;
    virtual std::string toString() const = 0;

    virtual Type* applyEnv(const SubstitutionTable& env, CompCtx_Ptr& ctx) const = 0;
    Type* applied(CompCtx_Ptr& ctx) const;

    const SubstitutionTable& getSubstitutionTable() const;

    static Type* NotYetDefined();

    static Type* findSubstitution(const SubstitutionTable& table, Type* toFind);

protected:

    static void applyEnvHelper(const SubstitutionTable& env, SubstitutionTable &to);

    const SubstitutionTable _subTable;
};

class ProperType : public Type {
public:
    ProperType(ast::ClassDecl* clss, const SubstitutionTable& substitutionTable = {});

    virtual ~ProperType();

    virtual TYPE_KIND getTypeKind() const override;
    virtual bool isSubTypeOf(const Type* other) const override;
    virtual std::string toString() const override;

    virtual Type* applyEnv(const SubstitutionTable& env, CompCtx_Ptr& ctx) const override;

    ast::ClassDecl* getClass() const;

private:

    ast::ClassDecl* _class;
};

class ConstructorType : public Type {
public:
    ConstructorType(ast::TypeConstructorCreation* typeConstructor, const SubstitutionTable& substitutionTable = {});

    virtual ~ConstructorType();

    virtual TYPE_KIND getTypeKind() const override;
    virtual bool isSubTypeOf(const Type* other) const override;
    virtual std::string toString() const override;

    virtual Type* applyEnv(const SubstitutionTable& env, CompCtx_Ptr& ctx) const override;

    ast::TypeConstructorCreation* getTypeConstructor() const;

private:

    ast::TypeConstructorCreation* _typeConstructor;
};

class ConstructorApplyType : public Type {
public:
    ConstructorApplyType(Type* callee, const std::vector<Type*>& args, const common::Positionnable& pos, const SubstitutionTable& substitutionTable = {});

    virtual ~ConstructorApplyType();

    virtual TYPE_KIND getTypeKind() const override;
    virtual bool isSubTypeOf(const Type* other) const override;
    virtual std::string toString() const override;

    virtual Type* applyEnv(const SubstitutionTable& env, CompCtx_Ptr& ctx) const override;

private:

    Type* _callee;
    const std::vector<Type*> _args;
    const common::Positionnable _pos;
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
inline ProperType* getIf(const Type* t) {
    return t->getTypeKind() == TYPE_PROPER ? (ProperType*)t : nullptr;
}

template<>
inline ConstructorType* getIf(const Type* t) {
    return t->getTypeKind() == TYPE_CONSTRUCTOR ? (ConstructorType*)t : nullptr;
}

template<>
inline ConstructorApplyType* getIf(const Type* t) {
    return t->getTypeKind() == TYPE_CONSTRUCTOR_APPLY ? (ConstructorApplyType*)t : nullptr;
}

}

}

#endif
