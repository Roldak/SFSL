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

KindExpression::~KindExpression() {

}

SFSL_AST_ON_VISIT_CPP(KindExpression)

ProperTypeKindSpecifier::ProperTypeKindSpecifier() {

}

ProperTypeKindSpecifier::~ProperTypeKindSpecifier() {

}

SFSL_AST_ON_VISIT_CPP(ProperTypeKindSpecifier)

TypeConstructorKindSpecifier::TypeConstructorKindSpecifier(const std::vector<KindExpression*>& args, KindExpression* ret)
    : _args(args), _ret(ret) {

}

TypeConstructorKindSpecifier::~TypeConstructorKindSpecifier() {

}

const std::vector<KindExpression*>& TypeConstructorKindSpecifier::getArgs() const {
    return _args;
}

KindExpression* TypeConstructorKindSpecifier::getRet() const {
    return _ret;
}

SFSL_AST_ON_VISIT_CPP(TypeConstructorKindSpecifier)

}

}
