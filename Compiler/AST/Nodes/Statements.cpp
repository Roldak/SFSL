//
//  Statements.cpp
//  SFSL
//
//  Created by Romain Beguet on 18.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Statements.h"
#include "../Visitors/ASTVisitor.h"

namespace sfsl {

namespace ast {

// STATEMENT

Statement::~Statement() {

}

SFSL_AST_ON_VISIT_CPP(Statement)

// EXPRESSION STATEMENT

ExpressionStatement::ExpressionStatement(Expression *expr) : _expr(expr) {

}

ExpressionStatement::~ExpressionStatement() {

}

SFSL_AST_ON_VISIT_CPP(ExpressionStatement)

Expression *ExpressionStatement::getExpression() const {
    return _expr;
}


// IF EXPRESSION


IfExpression::IfExpression(Expression *cond, Expression *then, Expression *els)
    : _cond(cond), _then(then), _else(els) {

}

IfExpression::~IfExpression() {

}

Expression *IfExpression::getCondition() const {
    return _cond;
}

Expression *IfExpression::getThen() const {
    return _then;
}

Expression *IfExpression::getElse() const {
    return _else;
}

SFSL_AST_ON_VISIT_CPP(IfExpression)

}

}
