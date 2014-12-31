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

namespace sfsl {

namespace sym {
    class ClassSymbol;
}

namespace type {

enum TYPE_KIND { TYPE_NYD, TYPE_OBJECT };

class Type {
public:
    virtual ~Type();

    virtual TYPE_KIND getTypeKind() = 0;
    virtual bool isSubTypeOf(Type* other) = 0;
    virtual std::string toString() = 0;

    static Type* NotYetDefined;
};

class ObjectType : public Type {
public:
    ObjectType(sym::ClassSymbol* clss);

    virtual ~ObjectType();

    virtual TYPE_KIND getTypeKind();
    virtual bool isSubTypeOf(Type* other);
    virtual std::string toString();

private:

    sym::ClassSymbol* _class;
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
