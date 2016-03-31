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

Symbol::Symbol(const std::string& name, const std::string& absoluteName)
    : _name(name), _absoluteName(absoluteName) {

}

Symbol::~Symbol() {

}

bool Symbol::isOverloadableWith(Symbol*) const {
    return false;
}

const std::string& Symbol::getName() const {
    return _name;
}

const std::string& Symbol::getAbsoluteName() const {
    return _absoluteName;
}

// MODULE SYMBOL

ModuleSymbol::ModuleSymbol(const std::string& name, const std::string& absoluteName) : Symbol(name, absoluteName) {

}

ModuleSymbol::~ModuleSymbol() {

}

SYM_TYPE ModuleSymbol::getSymbolType() const {
    return SYM_MODULE;
}

// CLASS SYMBOL

TypeSymbol::TypeSymbol(const std::string& name, const std::string& absoluteName, ast::TypeDecl* type)
    : Symbol(name, absoluteName), _type(type) {

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

DefinitionSymbol::DefinitionSymbol(const std::string& name, const std::string& absoluteName, ast::DefineDecl* def, ast::TypeExpression* owner)
    : Symbol(name, absoluteName), _def(def), _owner(owner), _overriden(nullptr) {

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

VariableSymbol::VariableSymbol(const std::string& name, const std::string& absoluteName) : Symbol(name, absoluteName) {

}

VariableSymbol::~VariableSymbol() {

}

SYM_TYPE VariableSymbol::getSymbolType() const {
    return SYM_VAR;
}

}

}
