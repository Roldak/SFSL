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

Scope::Scope(Scope* parent, bool isDefScope) : _parent(parent), _isDefScope(isDefScope) {

}

Scope::~Scope() {

}

Symbol* Scope::addSymbol(Symbol* sym) {
    auto res = _symbols.insert(std::map<std::string, Symbol*>::value_type(sym->getName(), sym));
    return res.second ? nullptr : (*res.first).second;
}

Scope* Scope::getParent() const {
    return _parent;
}

const std::map<std::string, Symbol*>& Scope::getAllSymbols() const {
    return _symbols;
}

Symbol* Scope::_getSymbol(const std::string& name, SYM_TYPE symType, bool recursive) const {
    if (_isDefScope && symType == SYM_VAR) {
        return nullptr;
    }

    const auto it = _symbols.find(name);

    if (it != _symbols.end() && (it->second->getSymbolType() == symType || symType == -1)) {
        return it->second;
    } else if (_parent && recursive) {
        return _parent->_getSymbol(name, symType, recursive);
    } else {
        return nullptr;
    }
}

}

}
