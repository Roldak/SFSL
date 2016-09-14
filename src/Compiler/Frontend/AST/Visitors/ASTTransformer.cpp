//
//  ASTTransformer.cpp
//  SFSL
//
//  Created by Romain Beguet on 20.07.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#include "ASTTransformer.h"
#include "../../../Common/CompilationContext.h"

namespace sfsl {

namespace ast {

ASTTransformer::ASTTransformer(CompCtx_Ptr& ctx) : ASTVisitor(ctx), _created(nullptr) {

}

ASTTransformer::~ASTTransformer() {

}

void ASTTransformer::visit(ASTNode*) {
    // do not throw an exception
}

void ASTTransformer::visit(Program* prog) {
    update(prog, transform<ModuleDecl>(prog->getModules()));
}

void ASTTransformer::visit(ModuleDecl* mod) {
    update(mod,
           transform<Identifier>(mod->getName()),
           transform<ModuleDecl>(mod->getSubModules()),
           transform<TypeDecl>(mod->getTypes()),
           transform<DefineDecl>(mod->getDeclarations()));
}

void ASTTransformer::visit(TypeDecl* tdecl) {
    update(tdecl,
           transform<TypeIdentifier>(tdecl->getName()),
           transform<TypeExpression>(tdecl->getExpression()),
           tdecl->isExtern());
}

void ASTTransformer::visit(ClassDecl* clss){
    update(clss, clss->getName(),
           transform<TypeExpression>(clss->getParent()),
           transform<TypeDecl>(clss->getTypeDecls()),
           transform<TypeSpecifier>(clss->getFields()),
           transform<DefineDecl>(clss->getDefs()),
           clss->isAbstract());
}

void ASTTransformer::visit(DefineDecl* decl) {
    update(decl,
           transform<Identifier>(decl->getName()),
           transform<TypeExpression>(decl->getTypeSpecifier()),
           transform<Expression>(decl->getValue()),
           decl->getFlags());
}

void ASTTransformer::visit(ProperTypeKindSpecifier* ptks) {
    update(ptks,
           transform<TypeExpression>(ptks->getLowerBoundExpr()),
           transform<TypeExpression>(ptks->getUpperBoundExpr()));
}

void ASTTransformer::visit(TypeConstructorKindSpecifier* tcks) {
    std::vector<TypeConstructorKindSpecifier::Parameter> newParams;
    for (TypeConstructorKindSpecifier::Parameter oldParam : tcks->getArgs()) {
        if (KindSpecifyingExpression* newKSE = transform<KindSpecifyingExpression>(oldParam.kindExpr, true)) {
            newParams.push_back(TypeConstructorKindSpecifier::Parameter(oldParam.varianceType, newKSE));
        }
    }
    update(tcks, tcks->getArgs(), transform<KindSpecifyingExpression>(tcks->getRet()));
}

void ASTTransformer::visit(FunctionTypeDecl* ftdecl) {
    update(ftdecl,
           transform<TypeExpression>(ftdecl->getTypeArgs()),
           transform<TypeExpression>(ftdecl->getArgTypes()),
           transform<TypeExpression>(ftdecl->getRetType()),
           transform<TypeExpression>(ftdecl->getClassEquivalent()));
}

void ASTTransformer::visit(TypeMemberAccess* tdot) {
    update(tdot,
           transform<TypeExpression>(tdot->getAccessed()),
           transform<TypeIdentifier>(tdot->getMember()));
}

void ASTTransformer::visit(TypeTuple* ttuple) {
    update(ttuple, transform<TypeExpression>(ttuple->getExpressions()));
}

void ASTTransformer::visit(TypeConstructorCreation* typeconstructor) {
    update(typeconstructor, typeconstructor->getName(),
           transform<TypeExpression>(typeconstructor->getArgs()),
           transform<TypeExpression>(typeconstructor->getBody()));
}

void ASTTransformer::visit(TypeConstructorCall* tcall) {
    update(tcall,
           transform<TypeExpression>(tcall->getCallee()),
           transform<TypeTuple>(tcall->getArgsTuple()));
}

void ASTTransformer::visit(TypeIdentifier* tident) {
    set(tident);
}

void ASTTransformer::visit(TypeToBeInferred* tbi) {
    set(tbi);
}

void ASTTransformer::visit(TypeParameter* tparam) {
    update(tparam, tparam->getVarianceType(),
           transform<TypeIdentifier>(tparam->getSpecified()),
           transform<KindSpecifyingExpression>(tparam->getKindNode()));
}

void ASTTransformer::visit(ExpressionStatement* exp) {
    update(exp, transform<Expression>(exp->getExpression()));
}

void ASTTransformer::visit(AssignmentExpression* aex) {
    update(aex,
           transform<Expression>(aex->getLhs()),
           transform<Expression>(aex->getRhs()));
}

void ASTTransformer::visit(TypeSpecifier* tps) {
    update(tps,
           transform<Identifier>(tps->getSpecified()),
           transform<TypeExpression>(tps->getTypeNode()));
}

void ASTTransformer::visit(Block* block) {
    update(block, transform<Expression>(block->getStatements()));
}

void ASTTransformer::visit(IfExpression* ifexpr) {
    update(ifexpr,
           transform<Expression>(ifexpr->getCondition()),
           transform<Expression>(ifexpr->getThen()),
           transform<Expression>(ifexpr->getElse()),
           ifexpr->isFromLazyOperator());
}

void ASTTransformer::visit(MemberAccess* dot) {
    update(dot,
           transform<Expression>(dot->getAccessed()),
           transform<Identifier>(dot->getMember()));
}

void ASTTransformer::visit(Tuple* tuple) {
    update(tuple, transform<Expression>(tuple->getExpressions()));
}

void ASTTransformer::visit(FunctionCreation* func) {
    update(func, func->getName(),
           transform<TypeTuple>(func->getTypeArgs()),
           transform<Expression>(func->getArgs()),
           transform<Expression>(func->getBody()),
           transform<TypeExpression>(func->getReturnType()));
}

void ASTTransformer::visit(FunctionCall* call) {
    update(call,
           transform<Expression>(call->getCallee()),
           transform<TypeTuple>(call->getTypeArgsTuple()),
           transform<Tuple>(call->getArgsTuple()));
}

void ASTTransformer::visit(Instantiation* inst) {
    update(inst, transform<TypeExpression>(inst->getInstantiatedExpression()));
}

void ASTTransformer::visit(Identifier* ident) {
    set(ident);
}

void ASTTransformer::visit(This* ths) {
    set(ths);
}

void ASTTransformer::visit(BoolLiteral* boollit) {
    set(boollit);
}

void ASTTransformer::visit(IntLiteral* intlit) {
    set(intlit);
}

void ASTTransformer::visit(RealLiteral* reallit) {
    set(reallit);
}

void ASTTransformer::visit(StringLiteral* stringlit) {
    set(stringlit);
}

}

}
