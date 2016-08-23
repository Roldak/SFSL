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

#include "../../Kinds/Kinds.h"
#include "../../Types/TypeParametrizable.h"
#include "../../Types/CanSubtype.h"

#include "../Utils/Annotations.h"
#include "../../Common/Miscellaneous.h"

namespace sfsl {

namespace ast {

class TypeIdentifier;
class KindSpecifyingExpression;

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
 *  - Its name
 *  - Its fields
 *  - Its definitions
 */
class ClassDecl :
        public TypeExpression,
        public sym::Scoped,
        public type::TypeParametrizable,
        public type::CanSubtype<ClassDecl*>,
        public common::HasManageableUserdata,
        public Annotable {
public:

    ClassDecl(
            const std::string& name,
            TypeExpression* parent,
            const std::vector<TypeDecl*>& tdecls,
            const std::vector<TypeSpecifier*>& fields,
            const std::vector<DefineDecl*>& defs,
            bool isAbstract);

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
     * @return The list of type declarations contained in this class
     */
    const std::vector<TypeDecl*>& getTypeDecls() const;

    /**
     * @return The list of fields declared in this class
     */
    const std::vector<TypeSpecifier*>& getFields() const;

    /**
     * @return The list of definitions declared in this class
     */
    const std::vector<DefineDecl*>& getDefs() const;

    /**
     * @return True if the class is marked with the `abstract` flag
     */
    bool isAbstract() const;

    typedef type::CanSubtype<ClassDecl*> CanSubtypeClasses;

private:

    std::string _name;
    TypeExpression* _parent;

    std::vector<TypeDecl*> _tdecls;
    std::vector<TypeSpecifier*> _fields;
    std::vector<DefineDecl*> _defs;

    bool _isAbstract;
};

/**
 * @brief Represents a function type declaration (e.g. (int, real)->int)
 */
class FunctionTypeDecl : public TypeExpression, public sym::Scoped {
public:

    FunctionTypeDecl(const std::vector<ast::TypeExpression*>& typeArgs, const std::vector<TypeExpression*>& argTypes,
                     TypeExpression* retType, TypeExpression* classEquivalent);
    virtual ~FunctionTypeDecl();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The TypeExpressions defining the type arguments
     */
    const std::vector<TypeExpression*>& getTypeArgs() const;

    /**
     * @return The TypeExpressions defining the types of the arguments
     */
    const std::vector<TypeExpression*>& getArgTypes() const;

    /**
     * @return The TypeExpression defining the return type of the function
     */
    TypeExpression* getRetType() const;

    /**
     * @return The TypeExpression equivalent to this node (e.g. Func1[int, int])
     */
    TypeExpression* getClassEquivalent() const;

    /**
     * @brief Constructors a FunctionTypeDecl with the given arguments. (will create the classEquivalent itself)
     * @return an instance of FunctionTypeDecl from the given arguments
     */
    static TypeExpression* make(const std::vector<TypeExpression*>& typeArgs, const std::vector<TypeExpression*>& argTypes,
                                TypeExpression* retType, const std::vector<std::string>& TCPath, CompCtx_Ptr ctx);

private:

    std::vector<TypeExpression*> _typeArgs;
    std::vector<TypeExpression*> _argTypes;
    TypeExpression* _retType;

    TypeExpression* _classEquivalent;
};

/**
 * @brief Represents a type member access (with a dot operation, e.g. `module.class`)
 */
class TypeMemberAccess : public TypeExpression, public sym::Symbolic<sym::Symbol> {
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
    const std::vector<TypeExpression*>& getExpressions() const;

private:

    std::vector<TypeExpression*> _exprs;
};

/**
 * @brief Represents a type constructor creation, e.g. `[T] => class { x: T; }`
 */
class TypeConstructorCreation :
        public TypeExpression,
        public sym::Scoped,
        public type::TypeParametrizable {
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
class TypeIdentifier : public TypeExpression, public sym::Symbolic<sym::Symbol> {
public:
    TypeIdentifier(const std::string& name);
    virtual ~TypeIdentifier();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The name of the identifier
     */
    const std::string& getValue() const;

private:

    std::string _name;
};

/**
 * @brief Represents the type expression used when no type is given in a local variable declaration.
 * e.g. x: = 2;
 *        ^
 */
class TypeToBeInferred : public TypeExpression {
public:
    TypeToBeInferred();
    virtual ~TypeToBeInferred();

    SFSL_AST_ON_VISIT_H
};

/**
 * @brief Represents a type parameter (e.g. `in T: *->*`)
 */
class TypeParameter : public TypeExpression {
public:
    TypeParameter(common::VARIANCE_TYPE varianceType, TypeIdentifier* specified, KindSpecifyingExpression* type);
    virtual ~TypeParameter();

    SFSL_AST_ON_VISIT_H

    /**
     * @return The variance annotation (`in`, `out`, or not specified)
     */
    common::VARIANCE_TYPE getVarianceType() const;

    /**
     * @return The specified part (e.g. `T`)
     */
    TypeIdentifier* getSpecified() const;

    /**
     * @return The kind part (e.g. `*->*`)
     */
    KindSpecifyingExpression* getKindNode() const;

private:

    common::VARIANCE_TYPE _varianceType;
    TypeIdentifier* _specified;
    KindSpecifyingExpression* _kind;
};

}

}

#endif
