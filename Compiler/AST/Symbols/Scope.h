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

namespace sfsl {

namespace sym {

    class Scope : public common::MemoryManageable {
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

        template<typename T>
        T* getSymbol(const std::string &name) const {
            return nullptr;
        }

        Scope* getParent() const;

    private:

        Symbol* _getSymbol(const std::string& name) const;
        Symbol* _getSymbol(const std::string& name, SYM_TYPE symType) const;

        Scope* _parent;
        bool _isDefScope;

        std::map<std::string, Symbol*> _symbols;

    };

    template<>
    inline Symbol* Scope::getSymbol(const std::string& name) const {
        return _getSymbol(name);
    }

    template<>
    inline ModuleSymbol* Scope::getSymbol(const std::string& name) const {
        return static_cast<ModuleSymbol*>(_getSymbol(name, SYM_MODULE));
    }

    template<>
    inline ClassSymbol* Scope::getSymbol(const std::string& name) const {
        return static_cast<ClassSymbol*>(_getSymbol(name, SYM_CLASS));
    }

    template<>
    inline DefinitionSymbol* Scope::getSymbol(const std::string& name) const {
        return static_cast<DefinitionSymbol*>(_getSymbol(name, SYM_DEF));
    }

    template<>
    inline VariableSymbol* Scope::getSymbol(const std::string& name) const {
        return static_cast<VariableSymbol*>(_getSymbol(name, SYM_VAR));
    }

}

}

#endif
