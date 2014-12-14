//
//  Scope.cpp
//  SFSL
//
//  Created by Romain Beguet on 21.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Scope.h"

namespace sfsl {

namespace sym {

Scope::Scope(Scope *parent, bool isDefScope) : _parent(parent), _isDefScope(isDefScope) {

}

Scope::~Scope() {

}

void Scope::addSymbol(const std::string &name, Symbol *sym) {
    _symbols[name] = sym;
}


Scope* Scope::getParent() const {
    return _parent;
}

Symbol* Scope::_getSymbol(const std::string &name) const {
    const auto it = _symbols.find(name);

    if (it != _symbols.end()) {
        return it->second;
    } else if (_parent) {
        return _parent->_getSymbol(name);
    } else {
        return nullptr;
    }
}

Symbol* Scope::_getSymbol(const std::string &name, SYM_TYPE symType) const {
    if (_isDefScope && symType == SYM_VAR) {
        return nullptr;
    }

    const auto it = _symbols.find(name);

    if (it != _symbols.end() && it->second->getSymbolType() == symType) {
        return it->second;
    } else if (_parent) {
        return _parent->_getSymbol(name, symType);
    } else {
        return nullptr;
    }
}

}

}
