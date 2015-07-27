//
//  ASTOverrideFinder.h
//  SFSL
//
//  Created by Romain Beguet on 27.07.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__ASTOverrideFinder__
#define __SFSL__ASTOverrideFinder__

#include <iostream>
#include "ASTVisitor.h"

namespace sfsl {

namespace ast {

/**
 * @brief
 */
class ASTOverrideFinder : public ASTVisitor {
public:

    /**
     * @brief Creates an ASTOverrideFinder
     * @param ctx the compilation context that will be used throughout the visits
     */
    ASTOverrideFinder(CompCtx_Ptr& ctx, sym::DefinitionSymbol* overridingSymbol);

    virtual ~ASTOverrideFinder();

    virtual void visit(ASTNode*) override;

    virtual void visit(ClassDecl* clss) override;

    /**
     * @return The symbol that was overriden, or nullptr if none were found
     */
    sym::DefinitionSymbol* getOverridenSymbol() const;

    /**
     * @brief Finds the symbol which the given node overrides
     *
     * @param node The node which overrides another one
     * @param ctx The compilation context
     * @return The symbol that is overriden by the given one (nullptr if none were found)
     */
    static sym::DefinitionSymbol* findOverridenSymbol(sym::DefinitionSymbol* node, CompCtx_Ptr& ctx);

protected:

    sym::DefinitionSymbol* _overridingSymbol;
    sym::DefinitionSymbol* _overridenSymbol;

    type::ProperType* _currentType;
};

}

}

#endif
