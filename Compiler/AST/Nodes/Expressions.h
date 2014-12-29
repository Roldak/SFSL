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
#include "../Symbols/Symbols.h"
#include "../Symbols/Symbolic.h"
#include "TypeNodes.h"

namespace sfsl {

namespace ast {

class Identifier;
class Statement;

/**
 * @brief A superclass that represents an expression.
 * Cannot be constructed. This class is there just to provide
 * a lower upper bound for all the expressions than ASTNode
 */
class Expression : virtual public ASTNode {
public:

    virtual ~Expression();

    SFSL_AST_ON_VISIT_H

};

/**
 * @brief Represents a binary expression, aka lhs oper rhs.
 */
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

/**
 * @brief Represents a type specifying expression, e.g `x: int`
 */
class TypeSpecifier : public Expression {
public:
    TypeSpecifier(Expression* specified, Expression* type);
    virtual ~TypeSpecifier();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The specified part
     */
    Expression* getSpecified() const;

    /**
     * @return The type part
     */
    Expression* getTypeNode() const;

private:

    Expression* _specified;
    Expression* _type;
};

/**
// * @brief Represents a member access (with a dot operation, e.g. `point.x`)
 */
class MemberAccess : public Expression {
public:

    MemberAccess(Expression* accessed, Identifier* member);
    virtual ~MemberAccess();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The accessed part (the left side)
     */
    Expression* getAccessed() const;

    /**
     * @return The member part (the right side)
     */
    Identifier* getMember() const;

private:

    Expression* _accessed;
    Identifier* _member;
};

/**
 * @brief Represents a tuple
 */
class Tuple : public Expression {
public:

    Tuple(const std::vector<Expression*>& exprs);
    virtual ~Tuple();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The sequence of expressions that compose the tuple
     */
    const std::vector<Expression*>& getExpressions();

private:

    const std::vector<Expression*> _exprs;

};

/**
 * @brief Represents a function creation, e.g. `(x) => 2 * x`
 */
class FunctionCreation : public Expression, public sym::Scoped {
public:

    FunctionCreation(Expression* args, Expression* body);
    virtual ~FunctionCreation();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The tuple of arguments
     */
    Expression* getArgs() const;

    /**
     * @return The body of the function
     */
    Expression* getBody() const;

private:

    Expression* _args;
    Expression* _body;
};

/**
 * @brief Represents a function call.
 */
class FunctionCall : public Expression {
public:

    FunctionCall(Expression* callee, Tuple* args);
    virtual ~FunctionCall();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The expression to which is applied the parentheses operator
     */
    Expression* getCallee() const;

    /**
     * @return The sequence of arguments which are applied to the callee
     */
    Tuple* getArgsTuple() const;

    /**
     * @return The arguments by extracting them directly from the tuple
     */
    const std::vector<Expression*>& getArgs() const;

private:

    Expression* _callee;
    Tuple* _args;

};

/**
 * @brief Represents an Identifier, which
 * refers to a symbol.
 */
class Identifier : public Expression, public sym::Symbolic<sym::Symbol> {
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
