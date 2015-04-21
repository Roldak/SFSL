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

    virtual TYPE_KIND getTypeKind() const {
        return TYPE_NYD;
    }

    virtual bool isSubTypeOf(const Type*) const {
        return false;
    }

    virtual std::string toString() const {
        return "<not yet defined>";
    }
};

Type::Type(const SubstitutionTable &substitutionTable) : _subTable(substitutionTable) {

}

Type::~Type() {

}

std::string Type::toString() const {
    std::string toRet = "";
    if (!_subTable.empty()) {
        toRet += "[";
        for (const auto& pair : _subTable) {
            toRet += pair.first->toString() + "=>" + pair.second->toString() + ", ";
        }
        toRet = toRet.substr(0, toRet.size() - 2) + "]";
    }
    return toRet;
}

Type* Type::NotYetDefined() {
    static TypeNotYetDefined nyd;
    return &nyd; // all we want is a unique memory area
}

const SubstitutionTable& Type::getSubstitutionTable() const {
    return _subTable;
}

Type* Type::trySubstitution(Type *type) const {
    return trySubstitution(_subTable, type);
}

Type* Type::trySubstitution(const SubstitutionTable& table, Type* type) {
    auto found = table.find(type);
    while (found != table.end()) {
        type = found->second;
        found = table.find(type);
    }
    return type;
}

// OBJECT TYPE

ObjectType::ObjectType(ast::ClassDecl* clss, const SubstitutionTable& substitutionTable)
    : Type(substitutionTable), _class(clss) {

}

ObjectType::~ObjectType() {

}

TYPE_KIND ObjectType::getTypeKind() const { return TYPE_OBJECT; }

bool ObjectType::isSubTypeOf(const Type* other) const {
    if (ObjectType* objother = getIf<ObjectType>(other)) {
        if (_class == objother->_class) { // TODO : change that when inheritance is supported.
            const SubstitutionTable& osub = objother->getSubstitutionTable();
            for (const auto& pair : _subTable) {
                const auto& subpair = osub.find(pair.first);
                if (!subpair->second->isSubTypeOf(pair.second)) { // TODO support contravariance maybe?
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

std::string ObjectType::toString() const {
    return _class->getName() + Type::toString();
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

TYPE_KIND ConstructorType::getTypeKind() const {
    return TYPE_CONSTRUCTOR;
}

bool ConstructorType::isSubTypeOf(const Type* other) const {
    return this == other;
}

std::string ConstructorType::toString() const {
    return "<type constructor>" + Type::toString();
}

ast::TypeConstructorCreation* ConstructorType::getTypeConstructor() const {
    return _typeConstructor;
}

// CONSTRUCTOR APPLY TYPE

ConstructorApplyType::ConstructorApplyType(ConstructorType *callee, const std::vector<Type *> &args)
    : _callee(callee), _args(args) {

}

ConstructorApplyType::~ConstructorApplyType() {

}

TYPE_KIND ConstructorApplyType::getTypeKind() const {
    return TYPE_CONSTRUCTOR_APPLY;
}

bool ConstructorApplyType::isSubTypeOf(const Type *other) const {
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
    return toRet + "]";
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
