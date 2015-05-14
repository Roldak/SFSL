//
//  ASTKindCreator.h
//  SFSL
//
//  Created by Romain Beguet on 14.05.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__ASTKindCreator__
#define __SFSL__ASTKindCreator__

#include <iostream>
#include "ASTVisitor.h"
#include "../../Types/Kinds/Kinds.h"

namespace sfsl {

namespace ast {

/**
 * @brief A visitor that can generate a kind from an ASTNode
 */
class ASTKindCreator : public ASTVisitor {
public:

    /**
     * @brief Creates an ASTKindCreator
     * @param ctx the compilation context that will be used throughout the visits
     */
    ASTKindCreator(CompCtx_Ptr& ctx);

    virtual ~ASTKindCreator();

    virtual void visit(ASTNode* node) override;

    virtual void visit(ProperTypeKindSpecifier* ptks) override;
    virtual void visit(TypeConstructorKindSpecifier* tcks) override;

    /**
     * @return The type created by the ASTTypeCreator
     */
    kind::Kind* getCreatedKind() const;

    /**
     * @brief Creates a kind from an ASTNode, if the node corresponds
     * to a valid syntax of a kind specifying node.
     *
     * @param node The node from which to create the kind
     * @param ctx The compilation context
     * @return The generated kind
     */
    static kind::Kind* createKind(ASTNode* node, CompCtx_Ptr& ctx);

protected:

    kind::Kind* _created;
};

}

}

#endif
