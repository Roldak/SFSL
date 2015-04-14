//
//  SymbolResolver.cpp
//  SFSL
//
//  Created by Romain Beguet on 14.04.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "SymbolResolver.h"
#include "Scope.h"

namespace sfsl {

namespace sym {

SymbolResolver::SymbolResolver(const ast::Program* prog) : _scope(prog->getScope()) {

}

Symbol* SymbolResolver::getSymbol(const std::string& fullPathName) {
    std::vector<std::string> parts;
    size_t i = 0, e = utils::split(parts, fullPathName, NAMESPACE_DELIMITER);
    Scope* scope = _scope;
    Symbol* lastSym = nullptr;

    for (; i < e; ++i) {
        if (!scope) {
            return nullptr;
        }

        if ((lastSym = scope->getSymbol<sym::Symbol>(parts[i], false))) {
            Scoped* scoped;

            switch (lastSym->getSymbolType()) {
                case SYM_CLASS:     scoped = static_cast<ClassSymbol*>(lastSym); break;
                case SYM_MODULE:    scoped = static_cast<ModuleSymbol*>(lastSym); break;
                default:            scoped = nullptr; break;
            }

            scope = scoped ? scoped->getScope() : nullptr;
        } else {
            return nullptr;
        }
    }

    return lastSym;
}

}

}
