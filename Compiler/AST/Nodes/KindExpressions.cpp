//
//  KindExpressions.cpp
//  SFSL
//
//  Created by Romain Beguet on 14.05.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "KindExpressions.h"
#include "../Visitors/ASTVisitor.h"

namespace sfsl {

namespace ast {

// TYPE EXPRESSION

KindSpecifyingExpression::~KindSpecifyingExpression() {

}

SFSL_AST_ON_VISIT_CPP(KindSpecifyingExpression)

ProperTypeKindSpecifier::ProperTypeKindSpecifier() {

}

ProperTypeKindSpecifier::~ProperTypeKindSpecifier() {

}

SFSL_AST_ON_VISIT_CPP(ProperTypeKindSpecifier)

TypeConstructorKindSpecifier::TypeConstructorKindSpecifier(const std::vector<KindSpecifyingExpression*>& args, KindSpecifyingExpression* ret)
    : _args(args), _ret(ret) {

}

TypeConstructorKindSpecifier::~TypeConstructorKindSpecifier() {

}

const std::vector<KindSpecifyingExpression*>& TypeConstructorKindSpecifier::getArgs() const {
    return _args;
}

KindSpecifyingExpression* TypeConstructorKindSpecifier::getRet() const {
    return _ret;
}

void TypeConstructorKindSpecifier::setRet(KindSpecifyingExpression *expr) {
    _ret = expr;
}

SFSL_AST_ON_VISIT_CPP(TypeConstructorKindSpecifier)

}

}
