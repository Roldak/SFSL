//
//  TypeExpressions.cpp
//  SFSL
//
//  Created by Romain Beguet on 23.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "TypeExpressions.h"
#include "../Visitors/ASTVisitor.h"

namespace sfsl {

namespace ast {

// TYPE EXPRESSION

TypeExpression::~TypeExpression() {

}

SFSL_AST_ON_VISIT_CPP(TypeExpression)

// CLASS DECLARATION

ClassDecl::ClassDecl(const std::string& name,
                     TypeExpression* parent,
                     const std::vector<TypeSpecifier*>& fields,
                     const std::vector<DefineDecl*>& defs)

    : _name(name), _parent(parent), _fields(fields), _defs(defs)
{

}

ClassDecl::~ClassDecl() {

}

SFSL_AST_ON_VISIT_CPP(ClassDecl)

const std::string& ClassDecl::getName() const {
    return _name;
}

TypeExpression* ClassDecl::getParent() const {
    return _parent;
}

const std::vector<TypeSpecifier*>& ClassDecl::getFields() const {
    return _fields;
}

const std::vector<DefineDecl*>& ClassDecl::getDefs() const{
    return _defs;
}

// TYPE TUPLE

TypeTuple::TypeTuple(const std::vector<TypeExpression*>& exprs) : _exprs(exprs) {

}

TypeTuple::~TypeTuple() {

}

SFSL_AST_ON_VISIT_CPP(TypeTuple)

const std::vector<TypeExpression*>& TypeTuple::getExpressions() {
    return _exprs;
}

// TYPE CONSTRUCTOR CREATION

TypeConstructorCreation::TypeConstructorCreation(const std::string &name, TypeTuple *args, Expression* body)
    : _name(name), _args(args), _body(body) {

}

TypeConstructorCreation::~TypeConstructorCreation() {

}

SFSL_AST_ON_VISIT_CPP(TypeConstructorCreation)

const std::string& TypeConstructorCreation::getName() const {
    return _name;
}

TypeTuple* TypeConstructorCreation::getArgs() const {
    return _args;
}

Expression* TypeConstructorCreation::getBody() const {
    return _body;
}

// TYPE CONSTRUCTOR CALL

TypeConstructorCall::TypeConstructorCall(Expression* callee, TypeTuple* args)
    : _callee(callee), _args(args) {

}

TypeConstructorCall::~TypeConstructorCall() {

}

SFSL_AST_ON_VISIT_CPP(TypeConstructorCall)

Expression* TypeConstructorCall::getCallee() const {
    return _callee;
}

const std::vector<TypeExpression*>& TypeConstructorCall::getArgs() const {
    return _args->getExpressions();
}

TypeTuple* TypeConstructorCall::getArgsTuple() const {
    return _args;
}

// TYPE IDENTIFIER

TypeIdentifier::TypeIdentifier(const std::string &name) : _name(name) {

}

TypeIdentifier::~TypeIdentifier() {

}

SFSL_AST_ON_VISIT_CPP(TypeIdentifier)

const std::string& TypeIdentifier::getValue() const {
    return _name;
}

}

}
