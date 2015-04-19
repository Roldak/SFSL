//
//  Types.cpp
//  SFSL
//
//  Created by Romain Beguet on 24.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Types.h"
#include "../AST/Nodes/TypeExpressions.h"

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

Type::Type(const SubstitutionTable &substitutionTable) : _subTable(substitutionTable) {

}

Type::~Type() {

}

Type* Type::NotYetDefined() {
    static TypeNotYetDefined nyd;
    return &nyd; // all we want is a unique memory area
}

const SubstitutionTable& Type::getSubstitutionTable() const {
    return _subTable;
}

Type* Type::trySubstitution(Type *type) const {
    auto found = _subTable.find(type);
    while (found != _subTable.end()) {
        type = found->second;
        found = _subTable.find(type);
    }
    return type;
}

// OBJECT TYPE

ObjectType::ObjectType(ast::ClassDecl* clss, const SubstitutionTable& substitutionTable)
    : Type(substitutionTable), _class(clss) {

}

ObjectType::~ObjectType() {

}

TYPE_KIND ObjectType::getTypeKind() { return TYPE_OBJECT; }

bool ObjectType::isSubTypeOf(Type* other) {
    if (ObjectType* objother = getIf<ObjectType>(other)) {
        if (_class == objother->_class) { // TODO : change that when inheritance is supported.

        }
    }
    return false;
}

std::string ObjectType::toString() {
    std::string toRet = _class->getName();
    if (!_subTable.empty()) {
        toRet += "[";
        for (const auto& pair : _subTable) {
            toRet += pair.first->toString() + "=>" + pair.second->toString() + ", ";
        }
        toRet = toRet.substr(0, toRet.size() - 2) + "]";
    }
    return toRet;
}

ast::ClassDecl* ObjectType::getClass() const {
    return _class;
}

// TYPE CONSTRUCTOR

ConstructorType::ConstructorType(ast::TypeConstructorCreation*typeConstructor, const SubstitutionTable& substitutionTable)
    : Type(substitutionTable), _typeConstructor(typeConstructor) {

}

ConstructorType::~ConstructorType() {

}

TYPE_KIND ConstructorType::getTypeKind() {
    return TYPE_CONSTRUCTOR;
}

bool ConstructorType::isSubTypeOf(Type *other) {
    return this == other;
}

std::string ConstructorType::toString() {
    return "<type constructor>";
}

ast::TypeConstructorCreation* ConstructorType::getTypeConstructor() const {
    return _typeConstructor;
}

// TYPED

Typed::Typed() : _type(Type::NotYetDefined()) {

}

Typed::~Typed() {

}

void Typed::setType(type::Type* type) {
    _type = type;
}

type::Type* Typed::type() const {
    return _type;
}

}

}
