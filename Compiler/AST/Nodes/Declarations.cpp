//
//  Declarations.cpp
//  SFSL
//
//  Created by Romain Beguet on 16.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Declarations.h"
#include "../Visitors/ASTVisitor.h"

namespace sfsl {

namespace ast {

// MODULE DECLARATION

ModuleDecl::ModuleDecl(Identifier *name,
                       const std::vector<ModuleDecl*>& mods,
                       const std::vector<sfsl::ast::ClassDecl*>& classes,
                       const std::vector<sfsl::ast::DefineDecl*>& decls)

    : _name(name), _mods(mods), _classes(classes), _decls(decls)
{

}

ModuleDecl::~ModuleDecl() {

}

SFSL_AST_ON_VISIT_CPP(ModuleDecl)

const std::vector<ModuleDecl*>& ModuleDecl::getSubModules() const {
    return _mods;
}

const std::vector<ClassDecl *> &ModuleDecl::getClasses() const {
    return _classes;
}

const std::vector<DefineDecl*>& ModuleDecl::getDeclarations() const {
    return _decls;
}

Identifier *ModuleDecl::getName() const {
    return _name;
}

// DEFINE DECLARATION

DefineDecl::DefineDecl(Identifier *name, ASTNode *value) : _name(name), _value(value) {

}

DefineDecl::~DefineDecl() {

}

SFSL_AST_ON_VISIT_CPP(DefineDecl)

Identifier *DefineDecl::getName() const {
    return _name;
}

ASTNode *DefineDecl::getValue() const {
    return _value;
}

// CLASS DECLARATION

ClassDecl::ClassDecl(Identifier *name,
                     const std::vector<TypeSpecifier *> fields,
                     const std::vector<DefineDecl *> defs)

    : _name(name), _fields(fields), _defs(defs)
{

}

ClassDecl::~ClassDecl() {

}

SFSL_AST_ON_VISIT_CPP(ClassDecl)

Identifier* ClassDecl::getName() const {
    return _name;
}

const std::vector<TypeSpecifier*>& ClassDecl::getFields() const {
    return _fields;
}

const std::vector<DefineDecl*>& ClassDecl::getDefs() const{
    return _defs;
}

}

}
