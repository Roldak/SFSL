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

namespace sfsl {

namespace ast {
    class ClassDecl;
    class TypeConstructorCreation;
}

namespace type {

enum TYPE_KIND { TYPE_NYD, TYPE_OBJECT, TYPE_CONSTRUCTOR };

class Type;

typedef std::map<Type*, Type*> SubstitutionTable;

class Type : public common::MemoryManageable {
public:
    Type(const SubstitutionTable& substitutionTable = {});

    virtual ~Type();

    virtual TYPE_KIND getTypeKind() const = 0;
    virtual bool isSubTypeOf(const Type* other) const = 0;
    virtual std::string toString() const = 0;

    static Type* NotYetDefined();

    const SubstitutionTable& getSubstitutionTable() const;

    Type* trySubstitution(Type* type) const;

    static Type* trySubstitution(const SubstitutionTable& table, Type *type);

protected:

    const SubstitutionTable _subTable;
};

class ObjectType : public Type {
public:
    ObjectType(ast::ClassDecl* clss, const SubstitutionTable& substitutionTable = {});

    virtual ~ObjectType();

    virtual TYPE_KIND getTypeKind() const;
    virtual bool isSubTypeOf(const Type* other) const;
    virtual std::string toString() const;

    ast::ClassDecl* getClass() const;

private:

    ast::ClassDecl* _class;
};

class ConstructorType : public Type {
public:
    ConstructorType(ast::TypeConstructorCreation* typeConstructor, const SubstitutionTable& substitutionTable = {});

    virtual ~ConstructorType();

    virtual TYPE_KIND getTypeKind() const;
    virtual bool isSubTypeOf(const Type* other) const;
    virtual std::string toString() const;

    ast::TypeConstructorCreation* getTypeConstructor() const;

private:

    ast::TypeConstructorCreation* _typeConstructor;
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
    void setType(type::Type* type);

    /**
     * @return The type of the object
     */
    type::Type* type() const;

protected:

    type::Type* _type;
};

template<typename T>
inline T* getIf(const Type* t) {
    return nullptr;
}

template<>
inline ObjectType* getIf(const Type* t) {
    return t->getTypeKind() == TYPE_OBJECT ? (ObjectType*)t : nullptr;
}

template<>
inline ConstructorType* getIf(const Type* t) {
    return t->getTypeKind() == TYPE_CONSTRUCTOR ? (ConstructorType*)t : nullptr;
}

}

}

#endif
