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
    : ASTExplicitVisitor(ctx), _isValid(false) {

}

ASTAssignmentChecker::~ASTAssignmentChecker() {

}

void ASTAssignmentChecker::visit(TypeSpecifier* tps) {
    tps->getSpecified()->onVisit(this);
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

void ASTAssignmentChecker::visit(Identifier* id) {
    _isValid = id->getSymbol()->getSymbolType() == sym::SYM_VAR;
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
