//
//  ASTAssignmentChecker.cpp
//  SFSL
//
//  Created by Romain Beguet on 23.07.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "ASTAssignmentChecker.h"

namespace sfsl {

namespace ast {

ASTAssignmentChecker::ASTAssignmentChecker(CompCtx_Ptr& ctx)
    : ASTVisitor(ctx), _isValid(false) {

}

ASTAssignmentChecker::~ASTAssignmentChecker() {

}

void ASTAssignmentChecker::visit(ASTNode*) {
    // do not throw an exception
}

void ASTAssignmentChecker::visit(Program*) {

}

void ASTAssignmentChecker::visit(ModuleDecl*) {

}

void ASTAssignmentChecker::visit(TypeDecl*) {

}

void ASTAssignmentChecker::visit(ClassDecl*){

}

void ASTAssignmentChecker::visit(DefineDecl*) {

}

void ASTAssignmentChecker::visit(ProperTypeKindSpecifier*) {

}

void ASTAssignmentChecker::visit(TypeConstructorKindSpecifier*) {

}

void ASTAssignmentChecker::visit(TypeMemberAccess*) {

}

void ASTAssignmentChecker::visit(TypeTuple*) {

}

void ASTAssignmentChecker::visit(TypeConstructorCreation*) {

}

void ASTAssignmentChecker::visit(TypeConstructorCall*) {

}

void ASTAssignmentChecker::visit(TypeIdentifier*) {

}

void ASTAssignmentChecker::visit(KindSpecifier*) {

}

void ASTAssignmentChecker::visit(ExpressionStatement*) {

}

void ASTAssignmentChecker::visit(BinaryExpression*) {

}

void ASTAssignmentChecker::visit(AssignmentExpression*) {

}

void ASTAssignmentChecker::visit(TypeSpecifier* tps) {
    tps->getSpecified()->onVisit(this);
}

void ASTAssignmentChecker::visit(Block*) {

}

void ASTAssignmentChecker::visit(IfExpression* ifexpr) {
    ifexpr->getThen()->onVisit(this);
    bool isThenValid = _isValid;

    if (!ifexpr->getElse()) {
        _isValid = false;
        return;
    }

    ifexpr->getElse()->onVisit(this);
    bool isElseValid = _isValid;

    _isValid = isThenValid && isElseValid;
}

void ASTAssignmentChecker::visit(MemberAccess* dot) {
    _isValid = dot->getSymbol()->getSymbolType() == sym::SYM_VAR;
}

void ASTAssignmentChecker::visit(Tuple* tuple) {
    for (Expression* expr : tuple->getExpressions()) {
        expr->onVisit(this);

        if (!_isValid) {
            return;
        }
    }
}

void ASTAssignmentChecker::visit(FunctionCreation*) {

}

void ASTAssignmentChecker::visit(FunctionCall*) {

}

void ASTAssignmentChecker::visit(Identifier* id) {
    _isValid = id->getSymbol()->getSymbolType() == sym::SYM_VAR;
}

void ASTAssignmentChecker::visit(This*) {

}

void ASTAssignmentChecker::visit(BoolLitteral*) {

}

void ASTAssignmentChecker::visit(IntLitteral*) {

}

void ASTAssignmentChecker::visit(RealLitteral*) {

}


bool ASTAssignmentChecker::isValid() const {
    return _isValid;
}

bool ASTAssignmentChecker::isExpressionAssignable(ASTNode* node, CompCtx_Ptr& ctx) {
    ASTAssignmentChecker checker(ctx);
    node->onVisit(&checker);
    return checker.isValid();
}

}

}
