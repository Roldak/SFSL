//
//  Symbols.h
//  SFSL
//
//  Created by Romain Beguet on 20.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Symbols__
#define __SFSL__Symbols__

#include <iostream>
#include <map>
#include "../../Common/MemoryManageable.h"
#include "../../Common/Positionnable.h"
#include "Scoped.h"

namespace sfsl {

namespace sym {

    enum SYM_TYPE{SYM_MODULE, SYM_CLASS, SYM_DEF, SYM_VAR};

    class ModuleSymbol;
    class ClassSymbol;
    class DefinitionSymbol;
    class VariableSymbol;

    class Symbol : public common::MemoryManageable, public common::Positionnable {
    public:

        Symbol(const std::string& name);
        virtual ~Symbol();

        virtual SYM_TYPE getSymbolType() const = 0;

        const std::string& getName() const;

    private:

        const std::string _name;
    };

    class ModuleSymbol : public Symbol, public Scoped {
    public:
        ModuleSymbol(const std::string& name);
        virtual ~ModuleSymbol();

        virtual SYM_TYPE getSymbolType() const;

        ModuleSymbol* getModule(const std::string& name) const;
        ClassSymbol* getClass(const std::string& name) const;
        DefinitionSymbol* getDefinition(const std::string& name) const;
    };

    class ClassSymbol : public Symbol {
    public:
        ClassSymbol(const std::string& name);
        virtual ~ClassSymbol();

        virtual SYM_TYPE getSymbolType() const;
    };

    class DefinitionSymbol : public Symbol {
    public:
        DefinitionSymbol(const std::string& name);
        virtual ~DefinitionSymbol();

        virtual SYM_TYPE getSymbolType() const;
    };

    class VariableSymbol : public Symbol {
    public:
        VariableSymbol(const std::string& name);
        virtual ~VariableSymbol();

        virtual SYM_TYPE getSymbolType() const;
    };

}

}

#endif
