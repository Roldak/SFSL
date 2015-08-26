//
//  ASTAssignmentChecker.h
//  SFSL
//
//  Created by Romain Beguet on 23.07.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__ASTAssignmentChecker__
#define __SFSL__ASTAssignmentChecker__

#include <iostream>
#include "ASTExplicitVisitor.h"

namespace sfsl {

namespace ast {

/**
 * @brief
 */
class ASTAssignmentChecker : public ASTExplicitVisitor {
public:

    /**
     * @brief Creates an ASTSymbolExtractor
     * @param ctx the compilation context that will be used throughout the visits
     */
    ASTAssignmentChecker(CompCtx_Ptr& ctx);

    virtual ~ASTAssignmentChecker();

    virtual void visit(TypeSpecifier* tps) override;
    virtual void visit(IfExpression* ifexpr) override;
    virtual void visit(MemberAccess* dot) override;
    virtual void visit(Tuple* tuple) override;
    virtual void visit(Identifier* ident) override;

    bool isValid() const;

    /**
     * @brief Checks if a given expression is assignable or not
     *
     * @param node The node to check
     * @param ctx The compilation context
     * @return true if the given node is assignable
     */
    static bool isExpressionAssignable(ASTNode* node, CompCtx_Ptr& ctx);

protected:

    bool _isValid;
};

}

}

#endif
