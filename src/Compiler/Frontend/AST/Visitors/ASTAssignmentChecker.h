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
    virtual void visit(MemberAccess* dot) override;
    virtual void visit(Tuple* tuple) override;
    virtual void visit(Identifier* ident) override;

    std::vector<sym::VariableSymbol*> get() const;

    /**
     * @brief Checks if a given expression is assignable or not,
     * and returns all the variable symbols that are assigned by
     * the expression. If there are 0, then the expression is not
     * assignable.
     *
     * @param node The node to check
     * @param ctx The compilation context
     * @return All the variable symbols that are assigned. (0 if expression is not valid)
     */
    static std::vector<sym::VariableSymbol*> getAssignedVars(ASTNode* node, CompCtx_Ptr& ctx);

protected:

    void visitSymbolic(sym::Symbolic<sym::Symbol>* symbolic);

    bool _isValid;
    std::vector<sym::VariableSymbol*> _assignedVars;
};

}

}

#endif
