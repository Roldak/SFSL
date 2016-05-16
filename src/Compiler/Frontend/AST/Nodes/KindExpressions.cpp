//
//  KindExpressions.cpp
//  SFSL
//
//  Created by Romain Beguet on 14.05.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "KindExpressions.h"
#include "../Visitors/ASTImplicitVisitor.h"

namespace sfsl {

namespace ast {

// KIND SPECITFYING EXPRESSION

KindSpecifyingExpression::~KindSpecifyingExpression() {

}

SFSL_AST_ON_VISIT_CPP(KindSpecifyingExpression)

// PROPER TYPE KIND SPECIFIER

ProperTypeKindSpecifier::ProperTypeKindSpecifier(TypeExpression* lowerBound, TypeExpression* upperBound)
    : _lb(lowerBound), _ub(upperBound) {

}

ProperTypeKindSpecifier::~ProperTypeKindSpecifier() {

}

TypeExpression* ProperTypeKindSpecifier::getLowerBoundExpr() const {
    return _lb;
}

TypeExpression* ProperTypeKindSpecifier::getUpperBoundExpr() const {
    return _ub;
}

SFSL_AST_ON_VISIT_CPP(ProperTypeKindSpecifier)

// TYPE CONSTRUCTOR KIND SPECIFIER

TypeConstructorKindSpecifier::Parameter::Parameter() : varianceType(common::VAR_T_NONE), kindExpr(nullptr) {

}

TypeConstructorKindSpecifier::Parameter::Parameter(common::VARIANCE_TYPE vt, KindSpecifyingExpression* expr) : varianceType(vt), kindExpr(expr) {

}

TypeConstructorKindSpecifier::Parameter::operator bool() const {
    return kindExpr;
}

TypeConstructorKindSpecifier::TypeConstructorKindSpecifier(const std::vector<Parameter>& args, KindSpecifyingExpression* ret)
    : _args(args), _ret(ret) {

}

TypeConstructorKindSpecifier::~TypeConstructorKindSpecifier() {

}

SFSL_AST_ON_VISIT_CPP(TypeConstructorKindSpecifier)

const std::vector<TypeConstructorKindSpecifier::Parameter>& TypeConstructorKindSpecifier::getArgs() const {
    return _args;
}

KindSpecifyingExpression* TypeConstructorKindSpecifier::getRet() const {
    return _ret;
}

void TypeConstructorKindSpecifier::setRet(KindSpecifyingExpression *expr) {
    _ret = expr;
}

}

}
