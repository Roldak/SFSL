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

    virtual void visit(MemberAccess* mac);
    virtual void visit(Identifier *ident);

    /**
     * @return The type created by the ASTTypeCreator
     */
    type::Type* getCreatedType() const;

protected:

    void createTypeFromSymbolic(sym::Symbolic<sym::Symbol>* symbolic, common::Positionnable& pos);

    type::Type* _created;

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

}

}

#endif
