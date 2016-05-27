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
#include "../AST/Utils/CanUseModules.h"

namespace sfsl {

namespace sym {

class Scope final : public common::MemoryManageable {
public:
    struct SymbolExcluder;

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
    void copySymbolsFrom(const Scope* other, const type::Environment& env, const SymbolExcluder* exluder);

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
    T* getSymbol(const std::string& name, bool recursive = true) const {
        return nullptr;
    }

    template<typename T>
    /**
     * @brief Looks up the symbol associated to the given name into this scope
     * or in a parent scope and assigns it to the given symbolic.
     *
     * @param symbolic The symbolic to assign
     * @param id The name of the symbol to look up
     * @return True if a symbol with the given named was found, otherwise false
     */
    bool assignSymbolic(sym::Symbolic<T>& symbolic, const std::string& id);

    /**
     * @return The parent if this scope
     */
    Scope* getParent() const;

    /**
     * @return The map containing all the symbols
     */
    const std::multimap<std::string, SymbolData>& getAllSymbols() const;

    /**
     * @brief Builds the related scopes from the using statements
     * given by the CanUseModules object.
     *
     * @param ctx The compilation context
     * @param obj The CanUseModules object which contains
     * the paths to the "used" modules
     */
    void buildUsingsFromPaths(CompCtx_Ptr& ctx, const ast::CanUseModules& obj);

    /**
     * @brief Abstract symbol excluder
     */
    struct SymbolExcluder {
        virtual ~SymbolExcluder();
        virtual bool exclude(const SymbolData s) const = 0;
    };

    /**
     * @brief Implementation of symbol excluder which excludes symbol based on the given name
     */
    struct ByNameSymbolExcluder : public SymbolExcluder {
        ByNameSymbolExcluder(const std::string& name);
        virtual ~ByNameSymbolExcluder();

        virtual bool exclude(const SymbolData s) const override;

    private:
        std::string _name;
    };

    /**
     * @brief Symbol excluder which excludes all constructors (symbols named "new")
     */
    static const SymbolExcluder* const ExcludeConstructors;

    /**
     * @brief Symbol excluder which doesn't exclude any symbol
     */
    static const SymbolExcluder* const KeepAll;

private:

    Symbol* _getSymbol(const std::string& name, SYM_TYPE symType, bool recursive, bool searchUsings) const;
    bool _assignSymbolicPrologue(Symbolic<Symbol>& symbolic, const std::string& id, bool searchUsings) const;
    bool _assignSymbolic(Symbolic<Symbol>& symbolic, const std::string& id, bool searchUsings) const;

    Scope* _parent;
    bool _isDefScope;

    std::vector<Scope*> _usedScopes;

    std::multimap<std::string, SymbolData> _symbols;
};

template<typename T>
bool Scope::assignSymbolic(Symbolic<T>& symbolic, const std::string& id) {
    return _assignSymbolicPrologue(static_cast<Symbolic<Symbol>&>(symbolic), id, true);
}

template<>
inline Symbol* Scope::getSymbol(const std::string& name, bool recursive) const {
    return _getSymbol(name, static_cast<SYM_TYPE>(-1), recursive, true);
}

template<>
inline ModuleSymbol* Scope::getSymbol(const std::string& name, bool recursive) const {
    return static_cast<ModuleSymbol*>(_getSymbol(name, SYM_MODULE, recursive, true));
}

template<>
inline TypeSymbol* Scope::getSymbol(const std::string& name, bool recursive) const {
    return static_cast<TypeSymbol*>(_getSymbol(name, SYM_TPE, recursive, true));
}

template<>
inline DefinitionSymbol* Scope::getSymbol(const std::string& name, bool recursive) const {
    return static_cast<DefinitionSymbol*>(_getSymbol(name, SYM_DEF, recursive, true));
}

template<>
inline VariableSymbol* Scope::getSymbol(const std::string& name, bool recursive) const {
    return static_cast<VariableSymbol*>(_getSymbol(name, SYM_VAR, recursive, true));
}

}

}

#endif
