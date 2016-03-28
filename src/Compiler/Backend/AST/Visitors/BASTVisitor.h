//
//  ASTImplicitVisitor.h
//  SFSL
//
//  Created by Romain Beguet on 28.03.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__BASTVisitor__
#define __SFSL__BASTVisitor__

#include <iostream>
#include "../Nodes/Nodes.h"
#include "../../../Common/CompilationContext.h"

namespace sfsl {

namespace bast {

/**
 * @brief An abstract class representing a visitor of BASTNodes.
 */
class BASTVisitor {
public:

    /**
     * @brief Creates a BASTVisitor
     * @param ctx the compilation context that will be used throughout the visits
     */
    BASTVisitor(CompCtx_Ptr& ctx);

    virtual ~BASTVisitor();

    virtual void visit(BASTNode* node) = 0;
    virtual void visit(Program* prog) = 0;

    virtual void visit(Expression* expr) = 0;
    virtual void visit(Block* block) = 0;
    virtual void visit(DefIdentifier* defid) = 0;
    virtual void visit(VarIdentifier* varid) = 0;
    virtual void visit(FieldAccess* fieldacc) = 0;
    virtual void visit(FieldAssignmentExpression* fassign) = 0;
    virtual void visit(VarAssignmentExpression* vassign) = 0;
    virtual void visit(IfExpression* ifexpr) = 0;
    virtual void visit(MethodCall* methcall) = 0;
    virtual void visit(Instantiation* inst) = 0;

    virtual void visit(BoolLitteral* boollit) = 0;
    virtual void visit(IntLitteral* intlit) = 0;
    virtual void visit(RealLitteral* reallit) = 0;
    virtual void visit(StringLitteral* strlit) = 0;


protected:

    CompCtx_Ptr _ctx;
    common::AbstractMemoryManager& _mngr;
};

}

}

#endif
