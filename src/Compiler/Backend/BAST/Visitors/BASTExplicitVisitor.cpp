//
//  BASTExplicitVisitor.cpp
//  SFSL
//
//  Created by Romain Beguet on on 28.03.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#include "BASTExplicitVisitor.h"

namespace sfsl {

namespace bast {

BASTExplicitVisitor::BASTExplicitVisitor(CompCtx_Ptr& ctx) : BASTVisitor(ctx) {

}

BASTExplicitVisitor::~BASTExplicitVisitor() {

}

void BASTExplicitVisitor::visit(BASTNode* node) {

}

void BASTExplicitVisitor::visit(Program* prog) {

}

void BASTExplicitVisitor::visit(Definition* def) {

}

void BASTExplicitVisitor::visit(MethodDef* meth) {

}

void BASTExplicitVisitor::visit(ClassDef* clss) {

}

void BASTExplicitVisitor::visit(GlobalDef* global) {

}

void BASTExplicitVisitor::visit(Expression* expr) {

}

void BASTExplicitVisitor::visit(Block* block) {

}

void BASTExplicitVisitor::visit(DefIdentifier* defid) {

}

void BASTExplicitVisitor::visit(VarIdentifier* varid) {

}

void BASTExplicitVisitor::visit(FieldAccess* fieldacc) {

}

void BASTExplicitVisitor::visit(FieldAssignmentExpression* fassign) {

}

void BASTExplicitVisitor::visit(VarAssignmentExpression* vassign) {

}

void BASTExplicitVisitor::visit(IfExpression* ifexpr) {

}

void BASTExplicitVisitor::visit(MethodCall* methcall) {

}

void BASTExplicitVisitor::visit(Instantiation* inst) {

}

void BASTExplicitVisitor::visit(BoolLitteral* boollit) {

}

void BASTExplicitVisitor::visit(IntLitteral* intlit) {

}

void BASTExplicitVisitor::visit(RealLitteral* reallit) {

}

void BASTExplicitVisitor::visit(StringLitteral* strlit) {

}

}

}
