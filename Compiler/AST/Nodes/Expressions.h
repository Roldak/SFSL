//
//  Expressions.h
//  SFSL
//
//  Created by Romain Beguet on 17.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Expressions__
#define __SFSL__Expressions__

#include <iostream>
#include <vector>
#include "../../../Utils/Utils.h"
#include "ASTNode.h"

namespace sfsl {

namespace ast {

class Identifier;

/**
 * @brief A superclass that represents an expression.
 * Cannot be constructed. This class is there just to provide
 * a lower upper bound for all the expressions than ASTNode
 */
class Expression : public ASTNode {
public:

    virtual ~Expression();

    SFSL_AST_ON_VISIT_H

};

class BinaryExpression : public Expression {
public:

    BinaryExpression(Expression* lhs, Expression* rhs, Identifier* oper);
    virtual ~BinaryExpression();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The left hand side of the binary expression
     */
    Expression* getLhs() const;

    /**
     * @return The right hand side of the binary expression
     */
    Expression* getRhs() const;

    /**
     * @return The operator of the binary expression
     */
    Identifier* getOperator() const;

private:

    Expression* _lhs;
    Expression* _rhs;
    Identifier* _oper;
};

class FunctionCall : public Expression {
public:

    FunctionCall(Expression* callee, const std::vector<Expression*>& args);
    virtual ~FunctionCall();

    SFSL_AST_ON_VISIT_H

    Expression* getCallee() const;
    const std::vector<Expression*>& getArgs() const;

private:

    Expression* _callee;
    const std::vector<Expression*> _args;

};

/**
 * @brief Represents an Identifier, which
 * refers to a symbol.
 */
class Identifier : public Expression {
public:

    Identifier(const std::string& name);
    virtual ~Identifier();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The name of the identifier
     */
    const std::string& getValue() const;

private:

    const std::string _name;

};

/**
 * @brief Represents an Integer litteral.
 */
class IntLitteral : public Expression {
public:

    IntLitteral(const sfsl_int_t value);
    virtual ~IntLitteral();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The value of the litteral
     */
    sfsl_int_t getValue() const;

private:

    const sfsl_int_t _value;
};

/**
 * @brief Represents a Real litteral
 */
class RealLitteral : public Expression {
public:

    RealLitteral(const sfsl_real_t value);
    virtual ~RealLitteral();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The value of the litteral
     */
    sfsl_real_t getValue() const;

private:

    const sfsl_real_t _value;
};

}

}

#endif
