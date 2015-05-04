//
//  TypeExpressions.h
//  SFSL
//
//  Created by Romain Beguet on 23.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__TypeExpressions__
#define __SFSL__TypeExpressions__

#include <iostream>
#include "Expressions.h"

namespace sfsl {

namespace ast {

/**
 * @brief The Class Declaration AST
 * Contains :
 *  - Its fields
 *  - Its definitions
 */
class ClassDecl : public Expression, public sym::Scoped {
public:

    ClassDecl(const std::string& name, Expression* parent, const std::vector<TypeSpecifier*>& fields, const std::vector<DefineDecl*>& defs);
    virtual ~ClassDecl();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The name of the class
     */
    const std::string& getName() const;

    /**
     * @return The expression defining the parent of this class
     */
    Expression* getParent() const;

    /**
     * @return The list of fields declared in this class
     */
    const std::vector<TypeSpecifier*>& getFields() const;

    /**
     * @return The list of definitions declared in this class
     */
    const std::vector<DefineDecl*>& getDefs() const;

private:

    std::string _name;
    Expression* _parent;
    std::vector<TypeSpecifier*> _fields;
    std::vector<DefineDecl*> _defs;
};

/**
 * @brief Represents a tuple
 */
class TypeTuple : public Expression {
public:

    TypeTuple(const std::vector<Expression*>& exprs);
    virtual ~TypeTuple();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The sequence of expressions that compose the tuple
     */
    const std::vector<Expression*>& getExpressions();

private:

    const std::vector<Expression*> _exprs;
};

/**
 * @brief Represents a type constructor creation, e.g. `[T] => class { x: T; }`
 */
class TypeConstructorCreation : public Expression, public sym::Scoped {
public:

    TypeConstructorCreation(const std::string& name, TypeTuple* args, Expression* body);
    virtual ~TypeConstructorCreation();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The name of the type constructor
     */
    const std::string& getName() const;

    /**
     * @return The tuple of arguments
     */
    TypeTuple* getArgs() const;

    /**
     * @return The body of the type constructor
     */
    Expression* getBody() const;

private:

    std::string _name;
    TypeTuple* _args;
    Expression* _body;
};

/**
 * @brief Represents a type constructor call.
 */
class TypeConstructorCall : public Expression {
public:

    TypeConstructorCall(Expression* callee, TypeTuple* args);
    virtual ~TypeConstructorCall();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The expression to which is applied the brackets operator
     */
    Expression* getCallee() const;

    /**
     * @return The sequence of arguments which are applied to the callee
     */
    TypeTuple* getArgsTuple() const;

    /**
     * @return The arguments by extracting them directly from the tuple
     */
    const std::vector<Expression*>& getArgs() const;

private:

    Expression* _callee;
    TypeTuple* _args;

};

}

}

#endif
