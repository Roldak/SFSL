//
//  Statements.h
//  SFSL
//
//  Created by Romain Beguet on 18.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Statements__
#define __SFSL__Statements__

#include <iostream>
#include "Expressions.h"

namespace sfsl {

namespace ast {

/**
 * @brief A superclass that represents a statement.
 * Cannot be constructed. This class is there just to provide
 * a lower upper bound for all the statements than ASTNode
 */
class Statement : virtual public ASTNode {
public:

    virtual ~Statement();

    SFSL_AST_ON_VISIT_H

};

/**
 * @brief Represents a statement witch is simply the evaluation of an expression
 */
class ExpressionStatement : public Statement {
public:

    ExpressionStatement(Expression* expr);
    virtual ~ExpressionStatement();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The expression contained in the statement
     */
    Expression* getExpression() const;

private:

    Expression* _expr;

};

/**
 * @brief Represents a block, aka a list of statements
 */
class Block : public Expression, public Statement {
public:
    Block(const std::vector<Statement*>& stats);
    virtual ~Block();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The list of statements contained in the block
     */
    const std::vector<Statement*>& getStatements() const;

private:

    const std::vector<Statement*> _stats;

};

/**
 * @brief Represents an If expression, e.g. if (1 + 1 == 2) println("hi") else println("salut")
 */
class IfExpression : public Expression, public Statement {
public:

    IfExpression(Expression* cond, ASTNode* then, ASTNode* els);
    virtual ~IfExpression();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The condition expression
     */
    Expression* getCondition() const;

    /**
     * @return The then-part expression
     */
    ASTNode* getThen() const;

    /**
     * @return The else-part expression, potentially null
     */
    ASTNode* getElse() const;

private:

    Expression* _cond;
    ASTNode* _then;
    ASTNode* _else;

};


}

}

#endif
