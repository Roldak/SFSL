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

#include "ASTNode.h"

#include "../../../../Utils/Utils.h"

#include "../../Symbols/Symbols.h"
#include "../../Symbols/Symbolic.h"

#include "../../Types/Types.h"
#include "../../Types/TypeParametrizable.h"

#include "../Utils/CanUseModules.h"
#include "../Utils/Annotations.h"

namespace sfsl {

namespace ast {

class Identifier;
class TypeIdentifier;
class TypeExpression;
class TypeTuple;

/**
 * @brief A superclass that represents an expression.
 * Cannot be constructed. This class is there just to provide
 * a lower upper bound for all the expressions than ASTNode
 */
class Expression : public ASTNode, public type::Typed {
public:

    virtual ~Expression();

    SFSL_AST_ON_VISIT_H

};

/**
 * @brief Represents a statement witch is simply the evaluation of an expression
 */
class ExpressionStatement : public Expression {
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
 * @brief The Define Declaration AST
 * Contains :
 *  - This definition's name
 *  - The value associated to this definition
 */
class DefineDecl :
        public Expression,
        public sym::Symbolic<sym::DefinitionSymbol>,
        public Annotable {
public:

    DefineDecl(Identifier* name, TypeExpression* tp, Expression* value, bool isRedef, bool isExtern, bool isAbstract);
    virtual ~DefineDecl();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The name of this definition
     */
    Identifier* getName() const;

    /**
     * @return The type specifying expression (nullptr if none were specified)
     */
    TypeExpression* getTypeSpecifier() const;

    /**
     * @return The value associated to this definition
     */
    Expression* getValue() const;

    /**
     * @return True if the definition is supposed to override another one
     */
    bool isRedef() const;

    /**
     * @return True if the definition is marked with the `extern` flag
     */
    bool isExtern() const;

    /**
     * @return True if the definition is marked with the `abstract` flag
     */
    bool isAbstract() const;

private:

    Identifier* _name;
    TypeExpression* _typeSpecifier;
    Expression* _value;

    bool _isRedef;
    bool _isExtern;
    bool _isAbstract;

};

class TypeDecl :
        public Expression,
        public sym::Symbolic<sym::TypeSymbol>,
        public Annotable {
public:

    TypeDecl(TypeIdentifier* id, TypeExpression* exp);
    virtual ~TypeDecl();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The name of the type
     */
    TypeIdentifier* getName() const;

    /**
     * @return The type expression
     */
    TypeExpression* getExpression() const;

private:

    TypeIdentifier* _name;
    TypeExpression* _exp;
};

/**
 * @brief Represents an assignement expression, aka lhs = rhs.
 */
class AssignmentExpression : public Expression {
public:

    AssignmentExpression(Expression* lhs, Expression* rhs);
    virtual ~AssignmentExpression();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The left hand side of the binary expression
     */
    Expression* getLhs() const;

    /**
     * @return The right hand side of the binary expression
     */
    Expression* getRhs() const;

private:

    Expression* _lhs;
    Expression* _rhs;
};

/**
 * @brief Represents a type specifying expression, e.g `x: int`
 */
class TypeSpecifier : public Expression {
public:
    TypeSpecifier(Identifier* specified, TypeExpression* type);
    virtual ~TypeSpecifier();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The specified part
     */
    Identifier* getSpecified() const;

    /**
     * @return The type part
     */
    TypeExpression* getTypeNode() const;

private:

    Identifier* _specified;
    TypeExpression* _type;
};

/**
 * @brief Represents a block, aka a list of statements
 */
class Block :
        public Expression,
        public sym::Scoped,
        public CanUseModules {
public:
    Block(const std::vector<Expression*>& stats);
    virtual ~Block();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The list of statements contained in the block
     */
    const std::vector<Expression*>& getStatements() const;

private:

    std::vector<Expression*> _stats;

};

/**
 * @brief Represents an If expression, e.g. if (1 + 1 == 2) println("hi") else println("salut")
 */
class IfExpression : public Expression {
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

/**
 * @brief Represents a member access (with a dot operation, e.g. `point.x`)
 */
class MemberAccess : public Expression, public sym::Symbolic<sym::Symbol> {
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

    std::vector<Expression*> _exprs;

};

/**
 * @brief Represents a function creation, e.g. `(x) => 2 * x`
 */
class FunctionCreation :
        public Expression,
        public sym::Scoped,
        public common::HasManageableUserdata,
        public type::TypeParametrizable,
        public Annotable {
public:

    FunctionCreation(const std::string& name, TypeTuple* typeArgs, Expression* args, Expression* body, TypeExpression* retType = nullptr);
    virtual ~FunctionCreation();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The name of the function
     */
    const std::string& getName() const;

    /**
     * @return The tuple of type arguments
     */
    TypeTuple* getTypeArgs() const;

    /**
     * @return The tuple of arguments
     */
    Expression* getArgs() const;

    /**
     * @return The body of the function
     */
    Expression* getBody() const;

    /**
     * @return The type expression specifying the return type (nullptr if none was specified)
     */
    TypeExpression* getReturnType() const;

private:

    std::string _name;
    TypeTuple* _typeArgs;
    Expression* _args;
    Expression* _body;
    TypeExpression* _retType;
};

/**
 * @brief Represents a function call.
 */
class FunctionCall : public Expression {
public:

    FunctionCall(Expression* callee, TypeTuple* typeArgs, Tuple* args);
    virtual ~FunctionCall();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The expression to which is applied the parentheses operator
     */
    Expression* getCallee() const;

    /**
     * @return The sequence of type arguments which are applied to the callee
     */
    TypeTuple* getTypeArgsTuple() const;

    /**
     * @return The sequence of arguments which are applied to the callee
     */
    Tuple* getArgsTuple() const;

    /**
     * @return The type arguments by extracting them directly from the tuple
     */
    const std::vector<TypeExpression*>& getTypeArgs() const;

    /**
     * @return The arguments by extracting them directly from the tuple
     */
    const std::vector<Expression*>& getArgs() const;

private:

    Expression* _callee;
    TypeTuple* _typeArgs;
    Tuple* _args;
};

/**
 * @brief Represents an instantiation of a type.
 * This node is never created by the parser.
 */
class Instantiation : public Expression {
public:

    Instantiation(TypeExpression* instantiated);
    virtual ~Instantiation();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The TypeExpression corresponding to the instantiated type
     */
    TypeExpression* getInstantiatedExpression() const;

private:

    TypeExpression* _instantiated;
};

/**
 * @brief Represents an Identifier, which refers to a symbol.
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

    std::string _name;
};

/**
 * @brief Represents the "this" identifier
 */
class This : public Expression {
public:

    This();
    virtual ~This();

    SFSL_AST_ON_VISIT_H
};

/**
 * @brief Represents a boolean Literal
 */
class BoolLiteral : public Expression {
public:

    BoolLiteral(const sfsl_bool_t value);
    virtual ~BoolLiteral();

    SFSL_AST_ON_VISIT_H

    sfsl_bool_t getValue() const;

private:

    sfsl_bool_t _value;
};

/**
 * @brief Represents an Integer Literal.
 */
class IntLiteral : public Expression {
public:

    IntLiteral(const sfsl_int_t value);
    virtual ~IntLiteral();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The value of the Literal
     */
    sfsl_int_t getValue() const;

private:

    sfsl_int_t _value;
};

/**
 * @brief Represents a Real Literal
 */
class RealLiteral : public Expression {
public:

    RealLiteral(const sfsl_real_t value);
    virtual ~RealLiteral();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The value of the Literal
     */
    sfsl_real_t getValue() const;

private:

    sfsl_real_t _value;
};

/**
 * @brief Represents a String Literal
 */
class StringLiteral : public Expression {
public:

    StringLiteral(const std::string& value);
    virtual ~StringLiteral();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The value of the Literal
     */
    const std::string& getValue() const;

private:

    std::string _value;
};

}

}

#endif
