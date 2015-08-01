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
    auto pos = _symbols.find(sym->getName());
    if (pos == _symbols.end() || pos->second.symbol->isOverloadableWith(sym)) {
        _symbols.insert(pos, make_pair(sym->getName(), SymbolData{.symbol = sym}));
        return nullptr;
    } else {
        return pos->second.symbol;
    }
}

void Scope::copySymbolsFrom(const Scope* other, const type::SubstitutionTable& env) {
    _symbols.insert(other->_symbols.begin(), other->_symbols.end());
    for (const std::pair<std::string, SymbolData>& entry : other->getAllSymbols()) {
        auto it = _symbols.insert(entry);
        type::Type::applyEnvHelper(env, it->second.env);
    }
}

Scope* Scope::getParent() const {
    return _parent;
}

const std::multimap<std::string, SymbolData>& Scope::getAllSymbols() const {
    return _symbols;
}

Symbol* Scope::_getSymbol(const std::string& name, SYM_TYPE symType, bool recursive) const {
    if (_isDefScope && symType == SYM_VAR) {
        return nullptr;
    }

    const auto it = _symbols.find(name);

    if (it != _symbols.end() && (it->second.symbol->getSymbolType() == symType || symType == -1)) {
        return it->second.symbol;
    } else if (_parent && recursive) {
        return _parent->_getSymbol(name, symType, recursive);
    } else {
        return nullptr;
    }
}

}

}
