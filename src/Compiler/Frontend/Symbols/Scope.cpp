//
//  Scope.cpp
//  SFSL
//
//  Created by Romain Beguet on 21.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Scope.h"
#include "../AST/Nodes/Expressions.h"

namespace sfsl {

namespace sym {

Scope::Scope(Scope* parent, bool isDefScope) : _parent(parent), _isDefScope(isDefScope) {

}

Scope::~Scope() {

}

Symbol* Scope::addSymbol(Symbol* sym) {
    auto pos = _symbols.find(sym->getName());
    if (pos == _symbols.end() || pos->second.symbol->isOverloadableWith(sym)) {
        _symbols.insert(pos, make_pair(sym->getName(), SymbolData(sym, type::Environment::Empty)));
        return nullptr;
    } else {
        return pos->second.symbol;
    }
}

void Scope::copySymbolsFrom(const Scope* other, const type::Environment& env, const SymbolExcluder* excluder) {
    for (const std::pair<std::string, SymbolData>& entry : other->getAllSymbols()) {
        if (excluder && excluder->exclude(entry.second)) {
            continue;
        }
        auto it = _symbols.insert(entry);
        it->second.env.substituteAll(env);
        it->second.env.insert(env.begin(), env.end());
    }
}

Scope* Scope::getParent() const {
    return _parent;
}

const std::multimap<std::string, SymbolData>& Scope::getAllSymbols() const {
    return _symbols;
}

void Scope::buildUsingsFromPaths(CompCtx_Ptr& ctx, const ast::CanUseModules& obj) {
    const std::vector<ast::CanUseModules::ModulePath>& paths(obj.getUsedModules());

    for (const ast::CanUseModules::ModulePath& path : paths) {
        ModuleSymbol* curMod = static_cast<ModuleSymbol*>(_getSymbol(path[0], sym::SYM_MODULE, true, false));
        if (!curMod) {
            ctx->reporter().error(path, "Cannot find any module named `" + path[0] + "`");
            break;
        }

        bool ok = true;
        for (size_t i = 1; i < path.size(); ++i) {
            if (ModuleSymbol* next = static_cast<ModuleSymbol*>(
                        curMod->getScope()->_getSymbol(path[i], sym::SYM_MODULE, false, false))) {
                curMod = next;
            } else {
                ok = false;
                ctx->reporter().error(path, "No module named `" + path[i] + "` inside module `" + path.toString(i) + "`");
                break;
            }
        }

        if (ok) {
            _usedScopes.push_back(curMod->getScope());
        }
    }
}

Symbol* Scope::_getSymbol(const std::string& name, SYM_TYPE symType, bool recursive, bool searchUsings) const {
    if (_isDefScope && symType == SYM_VAR) {
        return nullptr;
    }

    const auto it = _symbols.find(name);

    if (it != _symbols.end() && (it->second.symbol->getSymbolType() == symType || symType == -1)) {
        return it->second.symbol;
    } else if (recursive) {
        Symbol* found = _parent ? _parent->_getSymbol(name, symType, recursive, searchUsings) : nullptr;
        if (!found && searchUsings) {
            for (const Scope* s : _usedScopes) {
                if ((found = s->_getSymbol(name, symType, false, false))) {
                    break;
                }
            }
        }
        return found;
    } else {
        return nullptr;
    }
}

bool Scope::_assignSymbolicPrologue(sym::Symbolic<Symbol>& symbolic, const std::string& id, bool searchUsings) const {
    symbolic._symbols.clear();
    return _assignSymbolic(symbolic, id, searchUsings, false);
}

bool Scope::_assignSymbolic(sym::Symbolic<Symbol>& symbolic, const std::string& id, bool searchUsings, bool traversedDefScope) const {
    const auto& itPair = _symbols.equal_range(id);

    if (itPair.first != itPair.second) {
        for (auto it = itPair.first; it != itPair.second; ++it) {
            const SymbolData& data = it->second;

            // if we traversed a def scope, we can't access vars and method
            if (traversedDefScope) {
                if (data.symbol->getSymbolType() == sym::SYM_VAR) {
                    continue;
                } else if (sym::DefinitionSymbol* defsym = sym::getIfSymbolOfType<sym::DefinitionSymbol>(data.symbol)) {
                    if (defsym->getOwner() != nullptr) {
                        // means we must capture `this` which is also not possible
                        continue;
                    }
                }
            }

            symbolic._symbols.push_back(Symbolic<Symbol>::SymbolData(data.symbol, data.env));
        }
        return symbolic.getSymbolCount() > 0;
    } else if (searchUsings) {
        bool ok = false;

        for (const Scope* s : _usedScopes) {
            ok = ok | s->_assignSymbolic(symbolic, id, false, traversedDefScope);
        }

        if (!ok) { // couldn't find symbol in usings
            if (!(_parent && _parent->_assignSymbolic(symbolic, id, searchUsings, traversedDefScope | _isDefScope))) { // couldn't find symbol in parents
                return false;
            }
        }

        return true;
    }

    return false;
}

struct ConstructorExcluder : public Scope::SymbolExcluder {
    virtual ~ConstructorExcluder() {}
    virtual bool exclude(const SymbolData s) const override {
        if (sym::DefinitionSymbol* decl = sym::getIfSymbolOfType<sym::DefinitionSymbol>(s.symbol)) {
            return decl->getDef()->isConstructor();
        }
        return false;
    }
};

const Scope::SymbolExcluder* const Scope::ExcludeConstructors = new ConstructorExcluder();
const Scope::SymbolExcluder* const Scope::KeepAll = nullptr;

Scope::SymbolExcluder::~SymbolExcluder() {

}


Scope::ByNameSymbolExcluder::ByNameSymbolExcluder(const std::string& name) : _name(name) {

}

Scope::ByNameSymbolExcluder::~ByNameSymbolExcluder() {

}

bool Scope::ByNameSymbolExcluder::exclude(const SymbolData s) const {
    return s.symbol->getName() == _name;
}

}

}
