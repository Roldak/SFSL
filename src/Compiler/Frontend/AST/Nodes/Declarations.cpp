//
//  Declarations.cpp
//  SFSL
//
//  Created by Romain Beguet on 16.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Declarations.h"
#include "../Visitors/ASTImplicitVisitor.h"

namespace sfsl {

namespace ast {

// MODULE DECLARATION

ModuleDecl::ModuleDecl(Identifier *name,
                       const std::vector<ModuleDecl*>& mods,
                       const std::vector<TypeDecl*>& classes,
                       const std::vector<sfsl::ast::DefineDecl*>& decls)

    : _name(name), _mods(mods), _types(classes), _decls(decls)
{

}

ModuleDecl::~ModuleDecl() {

}

SFSL_AST_ON_VISIT_CPP(ModuleDecl)

const std::vector<ModuleDecl*>& ModuleDecl::getSubModules() const {
    return _mods;
}

const std::vector<TypeDecl*>& ModuleDecl::getTypes() const {
    return _types;
}

const std::vector<DefineDecl*>& ModuleDecl::getDeclarations() const {
    return _decls;
}

Identifier *ModuleDecl::getName() const {
    return _name;
}

// DEFINE DECLARATION

DefineDecl::DefineDecl(Identifier* name, Expression* value, bool isRedef, bool isExtern)
    : _name(name), _value(value), _isRedef(isRedef), _isExtern(isExtern) {

}

DefineDecl::~DefineDecl() {

}

SFSL_AST_ON_VISIT_CPP(DefineDecl)

Identifier* DefineDecl::getName() const {
    return _name;
}

Expression* DefineDecl::getValue() const {
    return _value;
}

bool DefineDecl::isRedef() const {
    return _isRedef;
}

bool DefineDecl::isExtern() const {
    return _isExtern;
}


// TYPE DECLARATION

TypeDecl::TypeDecl(TypeIdentifier *id, TypeExpression *exp) : _name(id), _exp(exp) {

}

TypeDecl::~TypeDecl() {

}

SFSL_AST_ON_VISIT_CPP(TypeDecl)

TypeIdentifier* TypeDecl::getName() const {
    return _name;
}

TypeExpression* TypeDecl::getExpression() const {
    return _exp;
}

}

}
