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

void ASTAssignmentChecker::visit(Identifier* node) {
    _isValid = node->getSymbol()->getSymbolType() == sym::SYM_VAR;
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
