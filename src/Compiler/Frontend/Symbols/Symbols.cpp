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

Symbol::Symbol(const std::string& name) : _name(name) {

}

Symbol::~Symbol() {

}

bool Symbol::isOverloadableWith(Symbol*) const {
    return false;
}

const std::string& Symbol::getName() const {
    return _name;
}

// MODULE SYMBOL

ModuleSymbol::ModuleSymbol(const std::string& name) : Symbol(name) {

}

ModuleSymbol::~ModuleSymbol() {

}

SYM_TYPE ModuleSymbol::getSymbolType() const {
    return SYM_MODULE;
}

// CLASS SYMBOL

TypeSymbol::TypeSymbol(const std::string& name, ast::TypeDecl* type) : Symbol(name), _type(type) {

}

TypeSymbol::~TypeSymbol() {

}

SYM_TYPE TypeSymbol::getSymbolType() const {
    return SYM_TPE;
}

ast::TypeDecl* TypeSymbol::getTypeDecl() const {
    return _type;
}

// DEFINITION SYMBOL

DefinitionSymbol::DefinitionSymbol(const std::string& name, ast::DefineDecl* def, ast::TypeExpression* owner)
    : Symbol(name), _def(def), _owner(owner), _overriden(nullptr) {

}

DefinitionSymbol::~DefinitionSymbol() {

}

SYM_TYPE DefinitionSymbol::getSymbolType() const {
    return SYM_DEF;
}

bool DefinitionSymbol::isOverloadableWith(Symbol* other) const {
    return other->getSymbolType() == SYM_DEF;
}

ast::DefineDecl* DefinitionSymbol::getDef() const {
    return _def;
}

ast::TypeExpression* DefinitionSymbol::getOwner() const {
    return _owner;
}

void DefinitionSymbol::setOverridenSymbol(DefinitionSymbol* def) {
    _overriden = def;
}

DefinitionSymbol *DefinitionSymbol::getOverridenSymbol() const {
    return _overriden;
}

// VARIABLE SYMBOL

VariableSymbol::VariableSymbol(const std::string& name) : Symbol(name) {

}

VariableSymbol::~VariableSymbol() {

}

SYM_TYPE VariableSymbol::getSymbolType() const {
    return SYM_VAR;
}

}

}
