//
//  ASTTypeCreator.h
//  SFSL
//
//  Created by Romain Beguet on 31.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__ASTTypeCreator__
#define __SFSL__ASTTypeCreator__

#include <iostream>
#include <set>
#include "ASTVisitor.h"
#include "../../Types/Types.h"

namespace sfsl {

namespace ast {

/**
 * @brief A visitor that can generate a type from an ASTNode
 */
class ASTTypeCreator : public ASTVisitor {
public:

    /**
     * @brief Creates an ASTTypeCreator
     * @param ctx the compilation context that will be used throughout the visits
     */
    ASTTypeCreator(CompCtx_Ptr& ctx);

    virtual ~ASTTypeCreator();

    virtual void visit(ASTNode* node);

    virtual void visit(ClassDecl* clss);

    virtual void visit(TypeConstructorCreation* typeconstructor);
    virtual void visit(TypeConstructorCall* tcall);

    virtual void visit(MemberAccess* mac);
    virtual void visit(Identifier *ident);

    /**
     * @return The type created by the ASTTypeCreator
     */
    type::Type* getCreatedType() const;

protected:

    void createTypeFromSymbolic(sym::Symbolic<sym::Symbol>* symbolic, common::Positionnable& pos);

    type::Type* _created;

    std::set<sym::TypeSymbol*> _visitedTypes;
};

/**
 * @brief Creates a type from an ASTNode, if the node corresponds
 * to a valid syntax of a type node.
 *
 * @param node The node from which to create the type
 * @param ctx The compilation context
 * @return The generated type
 */
inline type::Type* createType(ASTNode* node, CompCtx_Ptr& ctx) {
    ASTTypeCreator creator(ctx);
    node->onVisit(&creator);
    return creator.getCreatedType();
}

/**
 * @brief Evaluates the type of the TypeSymbol and returns the ClassDecl
 * associated to the ObjectType in case it evaluated to ObjectType,
 * otherwise returns nullptr
 *
 * @param sym The TypeSymbol from which to get the ClassDecl
 * @param ctx The compilation context
 * @return The ClassDecl is found, otherwise nullptr
 */
inline ast::ClassDecl* getClassDeclFromTypeSymbol(sym::TypeSymbol* sym, CompCtx_Ptr& ctx) {
    if (type::Type* t = createType(sym->getTypeDecl()->getExpression(), ctx)) {
        if (type::ObjectType* o = type::getIf<type::ObjectType>(t)) {
            return o->getClass();
        }
    }
    return nullptr;
}

}

}

#endif
