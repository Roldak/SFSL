//
//  ASTSymbolExtractor.h
//  SFSL
//
//  Created by Romain Beguet on 30.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__ASTSymbolExtractor__
#define __SFSL__ASTSymbolExtractor__

#include <iostream>
#include "ASTVisitor.h"

namespace sfsl {

namespace ast {

/**
 * @brief
 */
class ASTSymbolExtractor : public ASTVisitor {
public:

    /**
     * @brief Creates an ASTSymbolExtractor
     * @param ctx the compilation context that will be used throughout the visits
     */
    ASTSymbolExtractor(CompCtx_Ptr& ctx);

    virtual ~ASTSymbolExtractor();

    virtual void visit(ASTNode* node) override;

    virtual void visit(ModuleDecl* mod) override;
    virtual void visit(TypeDecl* type) override;
    virtual void visit(DefineDecl* def) override;

    virtual void visit(Identifier* id) override;
    virtual void visit(MemberAccess* mac) override;

    virtual void visit(TypeIdentifier* id) override;
    virtual void visit(TypeMemberAccess* mac) override;

    sym::Symbol* getSymbol() const;

protected:

    sym::Symbol* _sym;
};

inline sym::Symbol* extractSymbol(ASTNode* node, CompCtx_Ptr& ctx) {
    ASTSymbolExtractor extractor(ctx);
    node->onVisit(&extractor);
    return extractor.getSymbol();
}

}

}

#endif
