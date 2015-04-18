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

// CLASS DECLARATION

ClassDecl::ClassDecl(const std::string& name,
                     const std::vector<TypeSpecifier *> fields,
                     const std::vector<DefineDecl *> defs)

    : _name(name), _fields(fields), _defs(defs)
{

}

ClassDecl::~ClassDecl() {

}

SFSL_AST_ON_VISIT_CPP(ClassDecl)

const std::vector<TypeSpecifier*>& ClassDecl::getFields() const {
    return _fields;
}

const std::vector<DefineDecl*>& ClassDecl::getDefs() const{
    return _defs;
}

const std::string &ClassDecl::getName() const {
    return _name;
}

// TYPE TUPLE

TypeTuple::TypeTuple(const std::vector<Expression*>& exprs) : _exprs(exprs) {

}

TypeTuple::~TypeTuple() {

}

SFSL_AST_ON_VISIT_CPP(TypeTuple)

const std::vector<Expression*>& TypeTuple::getExpressions() {
    return _exprs;
}

// TYPE CONSTRUCTOR CREATION

TypeConstructorCreation::TypeConstructorCreation(TypeTuple *args, Expression* body) : _args(args), _body(body) {

}

TypeConstructorCreation::~TypeConstructorCreation() {

}

SFSL_AST_ON_VISIT_CPP(TypeConstructorCreation)

TypeTuple* TypeConstructorCreation::getArgs() const {
    return _args;
}

Expression* TypeConstructorCreation::getBody() const {
    return _body;
}

}

}
