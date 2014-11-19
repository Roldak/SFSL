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
 * @brief Represents an If expression, e.g. if (1 + 1 == 2) println("hi") else println("salut")
 */
class IfExpression : public Expression, public Statement {
public:

    IfExpression(Expression* cond, Expression* then, Expression* els);
    virtual ~IfExpression();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The condition expression
     */
    Expression* getCondition() const;

    /**
     * @return The then-part expression
     */
    Expression* getThen() const;

    /**
     * @return The else-part expression, potentially null
     */
    Expression* getElse() const;

private:

    Expression* _cond;
    Expression* _then;
    Expression* _else;

};


}

}

#endif
