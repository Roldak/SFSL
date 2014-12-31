//
//  Types.cpp
//  SFSL
//
//  Created by Romain Beguet on 24.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Types.h"
#include "../AST/Symbols/Symbols.h"

namespace sfsl {

namespace type {

class TypeNotYetDefined : public Type {
public:

    TypeNotYetDefined() {}

    virtual ~TypeNotYetDefined() {}

    virtual TYPE_KIND getTypeKind() {
        return TYPE_NYD;
    }

    virtual bool isSubTypeOf(Type *other) {
        return false;
    }

    virtual std::string toString() {
        return "<not yet defined>";
    }
};

Type::~Type() {

}

Type* Type::NotYetDefined = new TypeNotYetDefined();

ObjectType::ObjectType(sym::ClassSymbol *clss) : _class(clss) {

}

ObjectType::~ObjectType() {

}

TYPE_KIND ObjectType::getTypeKind() { return TYPE_OBJECT; }

bool ObjectType::isSubTypeOf(Type *other) {
    if (ObjectType* objother = getIf<ObjectType>(other)) {
        return _class == objother->_class;
    }
    return false;
}

std::string ObjectType::toString() {
    return _class->getName();
}

}

}
