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
#include "../../Common/Positionnable.h"
#include <iostream>
#include <map>

namespace sfsl {

namespace sym {

    enum SYM_TYPE{SYM_MODULE, SYM_CLASS, SYM_FUNC, SYM_VAR};

    class ModuleSymbol;
    class ClassSymbol;
    class FunctionSymbol;
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

    class ModuleSymbol : public Symbol {
    public:
        ModuleSymbol(const std::string& name);
        virtual ~ModuleSymbol();

        virtual SYM_TYPE getSymbolType() const;

        void addModule(ModuleSymbol* moduleSym);
        void addClass(ClassSymbol* classSym);
        void addFunction(FunctionSymbol* funcSym);

        ModuleSymbol* getModule(const std::string& name) const;
        ClassSymbol* getClass(const std::string& name) const;
        FunctionSymbol* getFunction(const std::string& name) const;

    private:

        std::map<std::string, ModuleSymbol*> _modules;
        std::map<std::string, ClassSymbol*> _classes;
        std::map<std::string, FunctionSymbol*> _functions;
    };

    class ClassSymbol : public Symbol {
    public:
        ClassSymbol(const std::string& name);
        virtual ~ClassSymbol();

        virtual SYM_TYPE getSymbolType() const;

        void addClass(ClassSymbol* classSym);
        void addFunction(FunctionSymbol* funcSym);
        void addAttribute(VariableSymbol* varSym);
    };

    class FunctionSymbol : public Symbol {
    public:
        FunctionSymbol(const std::string& name);
        virtual ~FunctionSymbol();

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
