//
//  ASTVisitor.cpp
//  SFSL
//
//  Created by Romain Beguet on 16.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "ASTVisitor.h"
#include "../../Common/CompilationContext.h"

namespace sfsl {

namespace ast {

ASTVisitor::ASTVisitor(std::shared_ptr<common::CompilationContext> &ctx) : _ctx(ctx) {

}

void ASTVisitor::visit(ASTNode *node) {
    throw common::CompilationFatalError("unimplemented visitor");
}

void ASTVisitor::visit(Program *prog) {
    for (ModuleDecl* module : prog->getModules()) {
        module->onVisit(this);
    }
}

void ASTVisitor::visit(ModuleDecl *module) {
    for (ASTNode* decl : module->getDeclarations()) {
        decl->onVisit(this);
    }
}

void ASTVisitor::visit(DefineDecl* decl) {
    decl->getName()->onVisit(this);
    decl->getValue()->onVisit(this);
}

void ASTVisitor::visit(BinaryExpression *bin) {
    bin->getLhs()->onVisit(this);
    bin->getOperator()->onVisit(this);
    bin->getRhs()->onVisit(this);
}

void ASTVisitor::visit(FunctionCall *call) {
    call->getCallee()->onVisit(this);

    for (auto arg : call->getArgs()) {
        arg->onVisit(this);
    }
}

void ASTVisitor::visit(Identifier* ident) {

}

void ASTVisitor::visit(IntLitteral* intlit) {

}

void ASTVisitor::visit(RealLitteral *reallit) {

}



}

}
