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
    ASTSymbolExtractor(std::shared_ptr<common::CompilationContext>& ctx);

    virtual ~ASTSymbolExtractor();

    virtual void visit(ASTNode* node);

    virtual void visit(ModuleDecl* mod);
    virtual void visit(ClassDecl* clss);
    virtual void visit(DefineDecl* def);

    virtual void visit(Identifier* id);
    virtual void visit(MemberAccess* mac);

    sym::Symbol* getSymbol() const;

protected:

    sym::Symbol* _sym;
};

inline sym::Symbol* extractSymbol(ASTNode* node, std::shared_ptr<common::CompilationContext>& ctx) {
    ASTSymbolExtractor extractor(ctx);
    node->onVisit(&extractor);
    return extractor.getSymbol();
}

}

}

#endif
