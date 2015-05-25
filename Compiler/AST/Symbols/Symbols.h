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
#include "../../Kinds/Kinds.h"
#include "../../../Utils/Utils.h"

namespace sfsl {

namespace ast {
    class DefineDecl;
    class TypeDecl;
}

namespace sym {

    enum SYM_TYPE{SYM_MODULE = 0, SYM_TPE, SYM_DEF, SYM_VAR};

    class ModuleSymbol;
    class TypeSymbol;
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

        virtual SYM_TYPE getSymbolType() const override;

        ModuleSymbol* getModule(const std::string& name) const;
        TypeSymbol* getClass(const std::string& name) const;
        DefinitionSymbol* getDefinition(const std::string& name) const;
    };

    /**
     * @brief Represents the symbol associated to a type
     */
    class TypeSymbol : public Symbol, public Scoped, public type::Typed, public kind::Kinded {
    public:
        TypeSymbol(const std::string& name, ast::TypeDecl* type);
        virtual ~TypeSymbol();

        virtual SYM_TYPE getSymbolType() const override;

        ast::TypeDecl* getTypeDecl() const;

    private:

        ast::TypeDecl* _type;
    };

    /**
     * @brief Represents the symbol associated to a definition
     */
    class DefinitionSymbol : public Symbol, public Scoped, public type::Typed {
    public:
        DefinitionSymbol(const std::string& name, ast::DefineDecl* def);
        virtual ~DefinitionSymbol();

        virtual SYM_TYPE getSymbolType() const override;

        ast::DefineDecl* getDef() const;

    private:

        ast::DefineDecl* _def;
    };

    /**
     * @brief Represents the symbol associated to a variable
     */
    class VariableSymbol : public Symbol, public type::Typed, public utils::UsageTrackable {
    public:
        VariableSymbol(const std::string& name);
        virtual ~VariableSymbol();

        virtual SYM_TYPE getSymbolType() const override;
    };

}

}

#endif
