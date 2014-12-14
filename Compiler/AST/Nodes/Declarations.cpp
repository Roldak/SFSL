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

ModuleDecl::ModuleDecl(Identifier *name, const std::vector<ModuleDecl*>& mods, const std::vector<DefineDecl*> decls)
    : _name(name), _mods(mods), _decls(decls) {

}

ModuleDecl::~ModuleDecl() {

}

const std::vector<ModuleDecl*>& ModuleDecl::getSubModules() const {
    return _mods;
}

const std::vector<DefineDecl*>& ModuleDecl::getDeclarations() const {
    return _decls;
}

Identifier *ModuleDecl::getName() const {
    return _name;
}

SFSL_AST_ON_VISIT_CPP(ModuleDecl)

// DEFINE DECLARATION

DefineDecl::DefineDecl(Identifier *name, ASTNode *value) : _name(name), _value(value) {

}

DefineDecl::~DefineDecl() {

}

Identifier *DefineDecl::getName() const {
    return _name;
}

ASTNode *DefineDecl::getValue() const {
    return _value;
}

SFSL_AST_ON_VISIT_CPP(DefineDecl)

}

}
