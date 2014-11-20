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

// BLOCK

Block::Block(const std::vector<Statement *> &stats) : _stats(stats) {

}

Block::~Block() {

}

SFSL_AST_ON_VISIT_CPP(Block)

const std::vector<Statement*>& Block::getStatements() const {
    return _stats;
}

// IF EXPRESSION


IfExpression::IfExpression(Expression* cond, ASTNode* then, ASTNode* els)
    : _cond(cond), _then(then), _else(els) {

}

IfExpression::~IfExpression() {

}

Expression *IfExpression::getCondition() const {
    return _cond;
}

ASTNode* IfExpression::getThen() const {
    return _then;
}

ASTNode* IfExpression::getElse() const {
    return _else;
}

SFSL_AST_ON_VISIT_CPP(IfExpression)

}

}
