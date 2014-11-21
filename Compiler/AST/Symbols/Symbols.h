//
//  Symbols.h
//  SFSL
//
//  Created by Romain Beguet on 20.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Symbols__
#define __SFSL__Symbols__

#include "../../Common/MemoryManageable.h"
#include <iostream>

namespace sfsl {

namespace sym {

    enum SYM_TYPE{SYM_MODULE, SYM_CLASS, SYM_FUNC, SYMC_VAR};

    class Symbol : public common::MemoryManageable {
    public:

        Symbol(const std::string& name);
        virtual ~Symbol();

        virtual SYM_TYPE getSymbolType() const = 0;

        const std::string& getName() const;

    private:

        const std::string _name;
    };

    class ModuleSymbol : public Symbol {
    public:
        ModuleSymbol(const std::string& name);
        virtual ~ModuleSymbol();

        virtual SYM_TYPE getSymbolType() const;
    };

    class ClassSymbol : public Symbol {
    public:
        ClassSymbol(const std::string& name);
        virtual ~ClassSymbol();

        virtual SYM_TYPE getSymbolType() const;
    };

    class FunctionSymbol : public Symbol {
    public:
        FunctionSymbol(const std::string& name);
        virtual ~FunctionSymbol();

        virtual SYM_TYPE getSymbolType() const;


    };

}

}

#endif
