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

ASTVisitor::ASTVisitor(CompCtx_Ptr &ctx) : _ctx(ctx), _mngr(ctx.get()->memoryManager()) {

}

ASTVisitor::~ASTVisitor() {

}

void ASTVisitor::visit(ASTNode* node) {
    throw common::CompilationFatalError("unimplemented visitor");
}

void ASTVisitor::visit(Program* prog) {
    for (ModuleDecl* module : prog->getModules()) {
        module->onVisit(this);
    }
}

void ASTVisitor::visit(ModuleDecl* module) {
    module->getName()->onVisit(this);

    for (ModuleDecl* mod : module->getSubModules()) {
        mod->onVisit(this);
    }
    for (TypeDecl* type : module->getTypes()) {
        type->onVisit(this);
    }
    for (DefineDecl* decl : module->getDeclarations()) {
        decl->onVisit(this);
    }
}

void ASTVisitor::visit(TypeDecl *tdecl) {
    tdecl->getName()->onVisit(this);
    tdecl->getExpression()->onVisit(this);
}

void ASTVisitor::visit(ClassDecl *clss) {
    for (TypeSpecifier* field : clss->getFields()) {
        field->onVisit(this);
    }
    for (DefineDecl* def : clss->getDefs()) {
        def->onVisit(this);
    }
}

void ASTVisitor::visit(DefineDecl* decl) {
    decl->getName()->onVisit(this);
    decl->getValue()->onVisit(this);
}

void ASTVisitor::visit(TypeTuple* ttuple) {
    for (auto arg : ttuple->getExpressions()) {
        arg->onVisit(this);
    }
}

void ASTVisitor::visit(TypeConstructorCreation *typeconstructor) {
    typeconstructor->getArgs()->onVisit(this);
    typeconstructor->getBody()->onVisit(this);
}

void ASTVisitor::visit(TypeConstructorCall *tcall) {
    tcall->getCallee()->onVisit(this);
    tcall->getArgsTuple()->onVisit(this);
}

void ASTVisitor::visit(ExpressionStatement* exp) {
    exp->getExpression()->onVisit(this);
}

void ASTVisitor::visit(BinaryExpression* bin) {
    bin->getLhs()->onVisit(this);
    bin->getOperator()->onVisit(this);
    bin->getRhs()->onVisit(this);
}

void ASTVisitor::visit(AssignmentExpression* aex) {
    aex->getLhs()->onVisit(this);
    aex->getRhs()->onVisit(this);
}

void ASTVisitor::visit(TypeSpecifier* tps) {
    tps->getSpecified()->onVisit(this);
    tps->getTypeNode()->onVisit(this);
}

void ASTVisitor::visit(Block* block) {
    for (auto stat : block->getStatements()) {
        stat->onVisit(this);
    }
}

void ASTVisitor::visit(IfExpression* ifexpr) {
    ifexpr->getCondition()->onVisit(this);
    ifexpr->getThen()->onVisit(this);

    if (ifexpr->getElse()) {
        ifexpr->getElse()->onVisit(this);
    }
}

void ASTVisitor::visit(MemberAccess* dot) {
    dot->getAccessed()->onVisit(this);
    dot->getMember()->onVisit(this);
}

void ASTVisitor::visit(Tuple* tuple) {
    for (auto arg : tuple->getExpressions()) {
        arg->onVisit(this);
    }
}

void ASTVisitor::visit(FunctionCreation* func) {
    func->getArgs()->onVisit(this);
    func->getBody()->onVisit(this);
}

void ASTVisitor::visit(FunctionCall* call) {
    call->getCallee()->onVisit(this);
    call->getArgsTuple()->onVisit(this);
}

void ASTVisitor::visit(Identifier* ident) {

}

void ASTVisitor::visit(IntLitteral* intlit) {

}

void ASTVisitor::visit(RealLitteral *reallit) {

}



}

}
