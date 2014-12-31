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
#include "../../Types/Types.h"

namespace sfsl {

namespace sym {

    enum SYM_TYPE{SYM_MODULE = 0, SYM_CLASS, SYM_DEF, SYM_VAR};

    class ModuleSymbol;
    class ClassSymbol;
    class DefinitionSymbol;
    class VariableSymbol;

    /**
     * @brief Abstract class representing a Symbol, which is a uniquely named object
     */
    class Symbol : public common::MemoryManageable, public common::Positionnable {
    public:
        virtual ~Symbol();

        /**
         * @return The type of this symbol
         */
        virtual SYM_TYPE getSymbolType() const = 0;

        /**
         * @return The name of this symbol
         */
        const std::string& getName() const;

    protected:
        Symbol(const std::string& name);

    private:

        const std::string _name;
    };

    /**
     * @brief Represents the symbol associated to a module.
     */
    class ModuleSymbol : public Symbol, public Scoped {
    public:
        ModuleSymbol(const std::string& name);
        virtual ~ModuleSymbol();

        virtual SYM_TYPE getSymbolType() const;

        ModuleSymbol* getModule(const std::string& name) const;
        ClassSymbol* getClass(const std::string& name) const;
        DefinitionSymbol* getDefinition(const std::string& name) const;
    };

    /**
     * @brief Represents the symbol associated to a class
     */
    class ClassSymbol : public Symbol, public Scoped {
    public:
        ClassSymbol(const std::string& name);
        virtual ~ClassSymbol();

        virtual SYM_TYPE getSymbolType() const;
    };

    /**
     * @brief Represents the symbol associated to a def
     */
    class DefinitionSymbol : public Symbol, public Scoped {
    public:
        DefinitionSymbol(const std::string& name);
        virtual ~DefinitionSymbol();

        virtual SYM_TYPE getSymbolType() const;
    };

    /**
     * @brief Represents the symbol associated to a variable
     */
    class VariableSymbol : public Symbol, public type::Typed {
    public:
        VariableSymbol(const std::string& name);
        virtual ~VariableSymbol();

        virtual SYM_TYPE getSymbolType() const;
    };

}

}

#endif
