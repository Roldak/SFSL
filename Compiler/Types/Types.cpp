//
//  Types.cpp
//  SFSL
//
//  Created by Romain Beguet on 24.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Types.h"
#include "../AST/Nodes/TypeExpressions.h"
#include "../AST/Visitors/ASTTypeCreator.h"
#include "../AST/Visitors/ASTTypeIdentifier.h"

namespace sfsl {

namespace type {

// TYPE NOT YET DEFINED

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

    virtual Type* applyEnv(const SubstitutionTable&, CompCtx_Ptr&) const {
        return Type::NotYetDefined();
    }

    virtual std::string toString() const {
        return "<not yet defined>";
    }
};

// TYPE

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

const SubstitutionTable& Type::getSubstitutionTable() const {
    return _subTable;
}

Type* Type::NotYetDefined() {
    static TypeNotYetDefined nyd;
    return &nyd; // all we want is a unique memory area
}

Type* Type::findSubstitution(const SubstitutionTable& table, Type* toFind) {
    auto found = table.find(toFind);
    while (found != table.end()) {
        toFind = found->second;
        found = table.find(toFind);
    }
    return toFind;
}

void Type::applyEnvHelper(const SubstitutionTable& env, SubstitutionTable& to) {
    for (auto& pair : to) {
        pair.second = findSubstitution(env, pair.second);
    }
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

Type* ObjectType::applyEnv(const SubstitutionTable& env, CompCtx_Ptr& ctx) const {
    SubstitutionTable table = _subTable;
    applyEnvHelper(env, table);
    return ctx.get()->memoryManager().New<ObjectType>(_class, table);
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
    return _typeConstructor->getName() + Type::toString();
}

Type* ConstructorType::applyEnv(const SubstitutionTable& env, CompCtx_Ptr& ctx) const {
    SubstitutionTable table = _subTable;
    applyEnvHelper(env, table);
    return ctx.get()->memoryManager().New<ConstructorType>(_typeConstructor, table);
}

ast::TypeConstructorCreation* ConstructorType::getTypeConstructor() const {
    return _typeConstructor;
}

// CONSTRUCTOR APPLY TYPE

ConstructorApplyType::ConstructorApplyType(Type* callee, const std::vector<Type*>& args,
                                           const common::Positionnable& pos,
                                           const SubstitutionTable& substitutionTable)
    : Type(substitutionTable), _callee(callee), _args(args), _pos(pos) {

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

Type* ConstructorApplyType::applyEnv(const SubstitutionTable& env, CompCtx_Ptr& ctx) const {
    Type* sub = findSubstitution(env, _callee)->applyEnv(env, ctx);

    if (ConstructorType* ctr = getIf<ConstructorType>(sub)) {
        const auto& params = ctr->getTypeConstructor()->getArgs()->getExpressions();
        SubstitutionTable subs;

        for (size_t i = 0; i < params.size(); ++i) {
            sym::TypeSymbol* param = nullptr;
            if (ast::isNodeOfType<ast::Identifier>(params[i], ctx)) {
                param = static_cast<sym::TypeSymbol*>(static_cast<ast::Identifier*>(params[i])->getSymbol());
            } else if (ast::isNodeOfType<ast::TypeConstructorCall>(params[i], ctx)) {
                // TODO : this code is ugly and most likely wrong
                param = static_cast<sym::TypeSymbol*>(
                            static_cast<ast::Identifier*>(
                                static_cast<ast::TypeConstructorCall*>(
                                    params[i]
                                )->getCallee()
                            )->getSymbol()
                        );
            }

            subs[param->type()] = findSubstitution(env, _args[i])->applyEnv(env, ctx);
        }


        return findSubstitution(subs, ast::createType(ctr->getTypeConstructor()->getBody(), ctx, subs))->applyEnv(subs, ctx);
    } else {
        ctx.get()->reporter().fatal(_pos, "Must have been a type constructor");
    }

    return Type::NotYetDefined();
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
