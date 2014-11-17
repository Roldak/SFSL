//
//  ASTVisitor.h
//  SFSL
//
//  Created by Romain Beguet on 16.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__ASTVisitor__
#define __SFSL__ASTVisitor__

#include <iostream>
#include "../Nodes/Program.h"
#include "../Nodes/Expressions.h"
#include "../../Common/CompilationContext.h"

namespace sfsl {

namespace ast {

/**
 * @brief An abstract class representing a visitor of ASTNodes.
 */
class ASTVisitor {
public:

    /**
     * @brief Creates an ASTVisitor
     * @param ctx the compilation context that will be used throughout the visits
     */
    ASTVisitor(std::shared_ptr<common::CompilationContext>& ctx);

    /**
     * @brief Should never be called.
     * @param node The visited node
     */
    virtual void visit(ASTNode* node);

    virtual void visit(Program* prog);

    virtual void visit(ModuleDecl* module);
    virtual void visit(DefineDecl* decl);

    virtual void visit(Identifier* ident);
    virtual void visit(IntLitteral* intlit);

private:

    std::shared_ptr<common::CompilationContext> _ctx;
};

}

}

#endif
