//
//  SymbolResolver.h
//  SFSL
//
//  Created by Romain Beguet on 14.04.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__SymbolResolver__
#define __SFSL__SymbolResolver__

#define NUMBER_OF_FUNC_TYPES 6

#include <iostream>
#include "../AST/Nodes/Program.h"
#include "../../../Common/PrimitiveNamer.h"

namespace sfsl {

namespace sym {

/**
 * @brief A class which makes it easy to retrieve symbols that are
 * defined in the AST
 */
class SymbolResolver final {
public:

    /**
     * @brief Creates a symbol resolver for the given program
     * @param prog The ast::Program for which to create the symbol resolver.
     * @param ctx The #sfsl::common::CompilationContext
     */
    SymbolResolver(const ast::Program* prog, const common::PrimitiveNamer& namer, const CompCtx_Ptr& ctx);

    ~SymbolResolver();

    /**
     * @brief Tries to retrieve the symbol for the given name.
     * @param fullPathName The full path to the symbol. Ex: sfsl.lang.int
     * @return A pointer to the #sfsl::sym::Symbol if found, otherwise nullptr.
     */
    Symbol* getSymbol(const std::vector<std::string>& fullPath) const;

    /**
     * @return The Unit type
     */
    type::Type* Unit() const;

    /**
     * @return The Bool type
     */
    type::Type* Bool() const;

    /**
     * @return The Int type
     */
    type::Type* Int() const;

    /**
     * @return The Real type
     */
    type::Type* Real() const;

    /**
     * @return The String type
     */
    type::Type* String() const;

    /**
     * @return The FuncX type, where X corresponds to the nbArgs argument passed
     */
    type::Type* Func(size_t nbArgs) const;

private:

    type::Type* createTypeFromSymbol(Symbol* sym);

    Scope* _scope;
    mutable CompCtx_Ptr _ctx;

    type::Type* _unitType;
    type::Type* _boolType;
    type::Type* _intType;
    type::Type* _realType;
    type::Type* _stringType;
    type::Type* _funcTypes[NUMBER_OF_FUNC_TYPES];
};

}

}

#endif
