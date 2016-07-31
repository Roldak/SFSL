//
//  Program.cpp
//  SFSL
//
//  Created by Romain Beguet on 16.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Program.h"
#include "../Visitors/ASTImplicitVisitor.h"

namespace sfsl {

namespace ast {

// PROGRAM

Program::Program(const std::vector<ModuleDecl*> &modules) : _modules(modules) {

}

const std::vector<ModuleDecl*>& Program::getModules() const {
    return _modules;
}

Program::~Program() {

}

SFSL_AST_ON_VISIT_CPP(Program)

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

}

}
