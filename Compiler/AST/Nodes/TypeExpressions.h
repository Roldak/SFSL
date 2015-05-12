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
#include "../../Types/Kinds/Kinds.h"

namespace sfsl {

namespace ast {

class TypeIdentifier;

/**
 * @brief A superclass that represents a type expression.
 * Cannot be constructed. This class is there just to provide
 * a lower upper bound for all the type expressions than Expression
 */
class TypeExpression : public Expression, public kind::Kinded {
public:

    virtual ~TypeExpression();

    SFSL_AST_ON_VISIT_H
};

/**
 * @brief The Class Declaration AST
 * Contains :
 *  - Its fields
 *  - Its definitions
 */
class ClassDecl : public TypeExpression, public sym::Scoped {
public:

    ClassDecl(const std::string& name, TypeExpression* parent, const std::vector<TypeSpecifier*>& fields, const std::vector<DefineDecl*>& defs);
    virtual ~ClassDecl();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The name of the class
     */
    const std::string& getName() const;

    /**
     * @return The expression defining the parent of this class
     */
    TypeExpression* getParent() const;

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
    TypeExpression* _parent;
    std::vector<TypeSpecifier*> _fields;
    std::vector<DefineDecl*> _defs;
};

/**
 * @brief Represents a type member access (with a dot operation, e.g. `module.class`)
 */
class TypeMemberAccess : public TypeExpression, public sym::Symbolic<sym::TypeSymbol> {
public:

    TypeMemberAccess(TypeExpression* accessed, TypeIdentifier* member);
    virtual ~TypeMemberAccess();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The accessed part (the left side)
     */
    TypeExpression* getAccessed() const;

    /**
     * @return The member part (the right side)
     */
    TypeIdentifier* getMember() const;

private:

    TypeExpression* _accessed;
    TypeIdentifier* _member;
};

/**
 * @brief Represents a tuple
 */
class TypeTuple : public TypeExpression {
public:

    TypeTuple(const std::vector<TypeExpression*>& exprs);
    virtual ~TypeTuple();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The sequence of expressions that compose the tuple
     */
    const std::vector<TypeExpression*>& getExpressions();

private:

    const std::vector<TypeExpression*> _exprs;
};

/**
 * @brief Represents a type constructor creation, e.g. `[T] => class { x: T; }`
 */
class TypeConstructorCreation : public TypeExpression, public sym::Scoped {
public:

    TypeConstructorCreation(const std::string& name, TypeExpression* args, TypeExpression* body);
    virtual ~TypeConstructorCreation();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The name of the type constructor
     */
    const std::string& getName() const;

    /**
     * @return The tuple of arguments
     */
    TypeExpression* getArgs() const;

    /**
     * @return The body of the type constructor
     */
    TypeExpression* getBody() const;

private:

    std::string _name;
    TypeExpression* _args;
    TypeExpression* _body;
};

/**
 * @brief Represents a type constructor call.
 */
class TypeConstructorCall : public TypeExpression {
public:

    TypeConstructorCall(TypeExpression* callee, TypeTuple* args);
    virtual ~TypeConstructorCall();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The expression to which is applied the brackets operator
     */
    TypeExpression* getCallee() const;

    /**
     * @return The sequence of arguments which are applied to the callee
     */
    TypeTuple* getArgsTuple() const;

    /**
     * @return The arguments by extracting them directly from the tuple
     */
    const std::vector<TypeExpression*>& getArgs() const;

private:

    TypeExpression* _callee;
    TypeTuple* _args;
};

/**
 * @brief Represents a type identifier, which
 * refers to a type symbol.
 */
class TypeIdentifier : public TypeExpression, public sym::Symbolic<sym::TypeSymbol> {
public:
    TypeIdentifier(const std::string& name);
    virtual ~TypeIdentifier();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The name of the identifier
     */
    const std::string& getValue() const;

private:

    const std::string _name;
};

// TODO : KindSpecifier

}

}

#endif
