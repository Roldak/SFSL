//
//  Symbols.cpp
//  SFSL
//
//  Created by Romain Beguet on 20.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Symbols.h"

namespace sfsl {

namespace sym {

// SYMBOL

Symbol::Symbol(const std::string &name) : _name(name) {

}

Symbol::~Symbol() {

}

const std::string& Symbol::getName() const {
    return _name;
}

// MODULE SYMBOL

ModuleSymbol::ModuleSymbol(const std::string &name) : Symbol(name) {

}

ModuleSymbol::~ModuleSymbol() {

}

SYM_TYPE ModuleSymbol::getSymbolType() const {
    return SYM_MODULE;
}

// CLASS SYMBOL

TypeSymbol::TypeSymbol(const std::string &name, ast::TypeDecl *type) : Symbol(name), _type(type) {

}

TypeSymbol::~TypeSymbol() {

}

SYM_TYPE TypeSymbol::getSymbolType() const {
    return SYM_TPE;
}

ast::TypeDecl* TypeSymbol::getType() const {
    return _type;
}

// DEFINITION SYMBOL

DefinitionSymbol::DefinitionSymbol(const std::string &name, ast::DefineDecl* def) : Symbol(name), _def(def) {

}

DefinitionSymbol::~DefinitionSymbol() {

}

SYM_TYPE DefinitionSymbol::getSymbolType() const {
    return SYM_DEF;
}

ast::DefineDecl *DefinitionSymbol::getDef() const {
    return _def;
}

// VARIABLE SYMBOL

VariableSymbol::VariableSymbol(const std::string &name) : Symbol(name) {

}

VariableSymbol::~VariableSymbol() {

}

SYM_TYPE VariableSymbol::getSymbolType() const {
    return SYM_VAR;
}

}

}
