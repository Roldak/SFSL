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
#include "../Nodes/Statements.h"
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

    virtual ~ASTVisitor();

    /**
     * @brief Should never be called.
     * @param node The visited node
     */
    virtual void visit(ASTNode* node);

    virtual void visit(Program* prog);

    virtual void visit(ModuleDecl* module);
    virtual void visit(ClassDecl* clss);
    virtual void visit(DefineDecl* decl);

    virtual void visit(ExpressionStatement* exp);

    virtual void visit(BinaryExpression* bin);
    virtual void visit(TypeSpecifier* tps);
    virtual void visit(Block* block);
    virtual void visit(IfExpression* ifexpr);
    virtual void visit(MemberAccess* dot);
    virtual void visit(Tuple* tuple);
    virtual void visit(FunctionCreation* func);
    virtual void visit(FunctionCall* call);
    virtual void visit(Identifier* ident);
    virtual void visit(IntLitteral* intlit);
    virtual void visit(RealLitteral* reallit);

protected:

    std::shared_ptr<common::CompilationContext> _ctx;
    common::AbstractMemoryManager& _mngr;
};

}

}

#endif
