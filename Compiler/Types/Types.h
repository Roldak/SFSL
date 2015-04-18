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

class Type : public common::MemoryManageable {
public:
    virtual ~Type();

    virtual TYPE_KIND getTypeKind() = 0;
    virtual bool isSubTypeOf(Type* other) = 0;
    virtual std::string toString() = 0;

    static Type* NotYetDefined();
};

typedef std::map<Type*, Type*> SubstitutionTable;

class ObjectType : public Type {
public:
    ObjectType(ast::ClassDecl* clss, const SubstitutionTable& substitutionTable = {});

    virtual ~ObjectType();

    virtual TYPE_KIND getTypeKind();
    virtual bool isSubTypeOf(Type* other);
    virtual std::string toString();

    ast::ClassDecl* getClass() const;
    const SubstitutionTable& getSubstitutionTable() const;

    Type* trySubsitution(Type* type) const;

private:

    ast::ClassDecl* _class;
    const SubstitutionTable _subTable;
};

class ConstructorType : public Type {
public:
    ConstructorType(ast::TypeConstructorCreation* typeConstructor);

    virtual ~ConstructorType();

    virtual TYPE_KIND getTypeKind();
    virtual bool isSubTypeOf(Type* other);
    virtual std::string toString();

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
inline T* getIf(Type* t) {
    return nullptr;
}

template<>
inline ObjectType* getIf(Type* t) {
    return t->getTypeKind() == TYPE_OBJECT ? (ObjectType*)t : nullptr;
}

}

}

#endif
