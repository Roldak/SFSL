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

        Scope(Scope* parent);
        virtual ~Scope();

        void setSymbol(const std::string& name, Symbol* sym);

        template<typename T>
        T* getSymbol(const std::string &name) const {
            return nullptr;
        }

    private:

        Symbol* _getSymbol(const std::string& name) const;
        Symbol* _getSymbol(const std::string& name, SYM_TYPE symType) const;

        Scope* _parent;

        std::map<std::string, Symbol*> _symbols;

    };

    template<>
    Symbol* Scope::getSymbol(const std::string& name) const {
        return _getSymbol(name);
    }

    template<>
    ModuleSymbol* Scope::getSymbol(const std::string& name) const {
        return static_cast<ModuleSymbol*>(_getSymbol(name, SYM_MODULE));
    }

    template<>
    ClassSymbol* Scope::getSymbol(const std::string& name) const {
        return static_cast<ClassSymbol*>(_getSymbol(name, SYM_CLASS));
    }

    template<>
    FunctionSymbol* Scope::getSymbol(const std::string& name) const {
        return static_cast<FunctionSymbol*>(_getSymbol(name, SYM_FUNC));
    }

    template<>
    VariableSymbol* Scope::getSymbol(const std::string& name) const {
        return static_cast<VariableSymbol*>(_getSymbol(name, SYM_VAR));
    }

}

}

#endif
