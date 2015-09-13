//
//  Scope.h
//  SFSL
//
//  Created by Romain Beguet on 21.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Scope__
#define __SFSL__Scope__

#include <iostream>
#include <map>
#include "../../Common/MemoryManageable.h"
#include "Symbols.h"
#include "Symbolic.h"

namespace sfsl {

namespace sym {

class Scope final : public common::MemoryManageable {
public:

    Scope(Scope* parent, bool isDefScope = false);
    virtual ~Scope();

    /**
     * @brief Adds the symbol in this scope unless a symbol of the same name
     * was already defined, in which case this symbol is returned
     *
     * @param sym The symbol to add
     * @return The symbol of the same name in the current scope if any, otherwise nullptr
     */
    Symbol* addSymbol(Symbol* sym);

    /**
     * @brief Copies all the symbols from the given scope into this one
     * @param other The scope from which to copy the symbol
     */
    void copySymbolsFrom(const Scope* other, const type::SubstitutionTable& env);

    template<typename T>
    /**
     * @brief Tries to retrieve the symbol with the given name in the current scope
     * and in the parent scopes with the recursive option. The type parameter
     * can be set to look for a specific Symbol type, or for any Symbol
     *
     * @param name The name of the Symbol to retrieve
     * @param recursive Sets whether or not to recursively look in the parents scope
     * @return The Symbol that matched the given arguments, or nullptr if none was found
     */
    T* getSymbol(const std::string &name, bool recursive = true) const {
        return nullptr;
    }

    template<typename T>
    bool assignSymbolic(sym::Symbolic<T>& symbolic, const std::string& id);

    /**
     * @return The parent if this scope
     */
    Scope* getParent() const;

    /**
     * @return The map containing all the symbols
     */
    const std::multimap<std::string, SymbolData>& getAllSymbols() const;

private:

    Symbol* _getSymbol(const std::string& name, SYM_TYPE symType, bool recursive) const;
    bool _assignSymbolic(Symbolic<Symbol>& symbolic, const std::string& id) const;

    Scope* _parent;
    bool _isDefScope;

    std::multimap<std::string, SymbolData> _symbols;
};

template<typename T>
bool Scope::assignSymbolic(Symbolic<T>& symbolic, const std::string& id) {
    return _assignSymbolic(static_cast<Symbolic<Symbol>&>(symbolic), id);
}

template<>
inline Symbol* Scope::getSymbol(const std::string& name, bool recursive) const {
    return _getSymbol(name, static_cast<SYM_TYPE>(-1), recursive);
}

template<>
inline ModuleSymbol* Scope::getSymbol(const std::string& name, bool recursive) const {
    return static_cast<ModuleSymbol*>(_getSymbol(name, SYM_MODULE, recursive));
}

template<>
inline TypeSymbol* Scope::getSymbol(const std::string& name, bool recursive) const {
    return static_cast<TypeSymbol*>(_getSymbol(name, SYM_TPE, recursive));
}

template<>
inline DefinitionSymbol* Scope::getSymbol(const std::string& name, bool recursive) const {
    return static_cast<DefinitionSymbol*>(_getSymbol(name, SYM_DEF, recursive));
}

template<>
inline VariableSymbol* Scope::getSymbol(const std::string& name, bool recursive) const {
    return static_cast<VariableSymbol*>(_getSymbol(name, SYM_VAR, recursive));
}

}

}

#endif