//
//  BASTImplicitVisitor.cpp
//  SFSL
//
//  Created by Romain Beguet on on 28.03.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#include "BASTImplicitVisitor.h"

namespace sfsl {

namespace bast {

BASTImplicitVisitor::BASTImplicitVisitor(CompCtx_Ptr& ctx) : BASTVisitor(ctx) {

}

BASTImplicitVisitor::~BASTImplicitVisitor() {

}

void BASTImplicitVisitor::visit(BASTNode* node) {

}

void BASTImplicitVisitor::visit(Program* prog) {
    for (const ClassDef& clss : prog->getClasses()) {
        for (const ClassDef::Method& meth : clss.getMethods()) {
            meth.getMethodBody()->onVisit(this);
        }
    }

    for (const GlobalDef& global : prog->getGlobals()) {
        global.getBody()->onVisit(this);
    }
}

void BASTImplicitVisitor::visit(Expression* expr) {

}

void BASTImplicitVisitor::visit(Block* block) {
    for (Expression* expr : block->getStatements()) {
        expr->onVisit(this);
    }
}

void BASTImplicitVisitor::visit(DefIdentifier* defid) {

}

void BASTImplicitVisitor::visit(VarIdentifier* varid) {

}

void BASTImplicitVisitor::visit(FieldAccess* fieldacc) {
    fieldacc->getAccessed()->onVisit(this);
}

void BASTImplicitVisitor::visit(FieldAssignmentExpression* fassign) {
    fassign->getAccessed()->onVisit(this);
    fassign->getValue()->onVisit(this);
}

void BASTImplicitVisitor::visit(VarAssignmentExpression* vassign) {
    vassign->getValue()->onVisit(this);
}

void BASTImplicitVisitor::visit(IfExpression* ifexpr) {
    ifexpr->getCondition()->onVisit(this);
    ifexpr->getThen()->onVisit(this);
    ifexpr->getElse()->onVisit(this);
}

void BASTImplicitVisitor::visit(MethodCall* methcall) {
    methcall->getCallee()->onVisit(this);
    for (Expression* arg : methcall->getArgs()) {
        arg->onVisit(this);
    }
}

void BASTImplicitVisitor::visit(Instantiation* inst) {

}

void BASTImplicitVisitor::visit(BoolLitteral* boollit) {

}

void BASTImplicitVisitor::visit(IntLitteral* intlit) {

}

void BASTImplicitVisitor::visit(RealLitteral* reallit) {

}

void BASTImplicitVisitor::visit(StringLitteral* strlit) {

}

}

}
