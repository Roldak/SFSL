//
//  ASTImplicitVisitor.cpp
//  SFSL
//
//  Created by Romain Beguet on 13.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "ASTImplicitVisitor.h"

namespace sfsl {

namespace ast {

ASTImplicitVisitor::ASTImplicitVisitor(CompCtx_Ptr &ctx) : ASTVisitor(ctx) {

}

ASTImplicitVisitor::~ASTImplicitVisitor() {

}

void ASTImplicitVisitor::visit(ASTNode*) {
    throw common::CompilationFatalError("unimplemented visitor");
}

void ASTImplicitVisitor::visit(Program* prog) {
    for (ModuleDecl* module : prog->getModules()) {
        module->onVisit(this);
    }
}

void ASTImplicitVisitor::visit(ModuleDecl* module) {
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

void ASTImplicitVisitor::visit(TypeDecl *tdecl) {
    tdecl->getName()->onVisit(this);
    tdecl->getExpression()->onVisit(this);
}

void ASTImplicitVisitor::visit(ClassDecl *clss) {
    if (clss->getParent()) {
        clss->getParent()->onVisit(this);
    }
    for (TypeDecl* tdecl: clss->getTypeDecls()) {
        tdecl->onVisit(this);
    }
    for (TypeSpecifier* field : clss->getFields()) {
        field->onVisit(this);
    }
    for (DefineDecl* def : clss->getDefs()) {
        def->onVisit(this);
    }
}

void ASTImplicitVisitor::visit(DefineDecl* decl) {
    decl->getName()->onVisit(this);
    if (TypeExpression* expr = decl->getTypeSpecifier()) {
        expr->onVisit(this);
    }
    if (Expression* val = decl->getValue()) {
        val->onVisit(this);
    }
}

void ASTImplicitVisitor::visit(ProperTypeKindSpecifier* ptks) {

}

void ASTImplicitVisitor::visit(TypeConstructorKindSpecifier* tcks) {
    for (KindSpecifyingExpression* arg : tcks->getArgs()) {
        arg->onVisit(this);
    }
    tcks->getRet()->onVisit(this);
}

void ASTImplicitVisitor::visit(FunctionTypeDecl* ftdecl) {
    for (TypeExpression* arg : ftdecl->getArgTypes()) {
        arg->onVisit(this);
    }
    ftdecl->getRetType()->onVisit(this);
}

void ASTImplicitVisitor::visit(TypeMemberAccess* tdot) {
    tdot->getAccessed()->onVisit(this);
    tdot->getMember()->onVisit(this);
}

void ASTImplicitVisitor::visit(TypeTuple* ttuple) {
    for (auto arg : ttuple->getExpressions()) {
        arg->onVisit(this);
    }
}

void ASTImplicitVisitor::visit(TypeConstructorCreation* typeconstructor) {
    typeconstructor->getArgs()->onVisit(this);
    typeconstructor->getBody()->onVisit(this);
}

void ASTImplicitVisitor::visit(TypeConstructorCall* tcall) {
    tcall->getCallee()->onVisit(this);
    tcall->getArgsTuple()->onVisit(this);
}

void ASTImplicitVisitor::visit(TypeIdentifier*) {

}

void ASTImplicitVisitor::visit(TypeToBeInferred*) {

}

void ASTImplicitVisitor::visit(KindSpecifier* ks) {
    ks->getSpecified()->onVisit(this);
    ks->getKindNode()->onVisit(this);
}

void ASTImplicitVisitor::visit(ExpressionStatement* exp) {
    exp->getExpression()->onVisit(this);
}

void ASTImplicitVisitor::visit(AssignmentExpression* aex) {
    aex->getLhs()->onVisit(this);
    aex->getRhs()->onVisit(this);
}

void ASTImplicitVisitor::visit(TypeSpecifier* tps) {
    tps->getSpecified()->onVisit(this);
    tps->getTypeNode()->onVisit(this);
}

void ASTImplicitVisitor::visit(Block* block) {
    for (auto stat : block->getStatements()) {
        stat->onVisit(this);
    }
}

void ASTImplicitVisitor::visit(IfExpression* ifexpr) {
    ifexpr->getCondition()->onVisit(this);
    ifexpr->getThen()->onVisit(this);

    if (Expression* expr = ifexpr->getElse()) {
        expr->onVisit(this);
    }
}

void ASTImplicitVisitor::visit(MemberAccess* dot) {
    dot->getAccessed()->onVisit(this);
    dot->getMember()->onVisit(this);
}

void ASTImplicitVisitor::visit(Tuple* tuple) {
    for (auto arg : tuple->getExpressions()) {
        arg->onVisit(this);
    }
}

void ASTImplicitVisitor::visit(FunctionCreation* func) {
    func->getArgs()->onVisit(this);

    if (TypeExpression* retType = func->getReturnType()) {
        retType->onVisit(this);
    }

    func->getBody()->onVisit(this);
}

void ASTImplicitVisitor::visit(FunctionCall* call) {
    call->getCallee()->onVisit(this);
    call->getArgsTuple()->onVisit(this);
}

void ASTImplicitVisitor::visit(Identifier*) {

}

void ASTImplicitVisitor::visit(This*) {

}

void ASTImplicitVisitor::visit(BoolLitteral*) {

}

void ASTImplicitVisitor::visit(IntLitteral*) {

}

void ASTImplicitVisitor::visit(RealLitteral*) {

}

void ASTImplicitVisitor::visit(StringLitteral*) {

}



}

}
