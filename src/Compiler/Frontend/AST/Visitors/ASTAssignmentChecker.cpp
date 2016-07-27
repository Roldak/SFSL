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
    : ASTExplicitVisitor(ctx), _isValid(true) {

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

    _isValid &= isThenValid & isElseValid;
}

void ASTAssignmentChecker::visit(MemberAccess* dot) {
    visitSymbolic(dot);
}

void ASTAssignmentChecker::visit(Tuple* tuple) {
    for (Expression* expr : tuple->getExpressions()) {
        expr->onVisit(this);

        if (!_isValid) {
            return;
        }
    }
}

void ASTAssignmentChecker::visit(Identifier* ident) {
    visitSymbolic(ident);
}

void ASTAssignmentChecker::visitSymbolic(sym::Symbolic<sym::Symbol>* symbolic) {
    if (symbolic->getSymbol()) {
        if (sym::VariableSymbol* var = sym::getIfSymbolOfType<sym::VariableSymbol>(symbolic->getSymbol())) {
            _assignedVars.push_back(var);
            return;
        }
    }
    _isValid = false;
}

std::vector<sym::VariableSymbol*> ASTAssignmentChecker::get() const {
    if (_isValid) {
        return _assignedVars;
    } else {
        return std::vector<sym::VariableSymbol*>();
    }
}

std::vector<sym::VariableSymbol*> ASTAssignmentChecker::getAssignedVars(ASTNode* node, CompCtx_Ptr& ctx) {
    ASTAssignmentChecker checker(ctx);
    node->onVisit(&checker);
    return checker.get();
}

}

}
