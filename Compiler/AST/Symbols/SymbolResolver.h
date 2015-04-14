//
//  SymbolResolver.h
//  SFSL
//
//  Created by Romain Beguet on 14.04.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__SymbolResolver__
#define __SFSL__SymbolResolver__

#define NAMESPACE_DELIMITER '.'

#include <iostream>
#include "../../AST/Nodes/Program.h"

namespace sfsl {

namespace sym {


/**
 * @brief A class which makes it easy to retrieve symbols that are
 * defined in the AST
 */
class SymbolResolver {
public:

    /**
     * @brief Creates a symbol resolver for the given program
     * @param prog The ast::Program for which to create the symbol resolver.
     */
    SymbolResolver(const ast::Program* prog);

    /**
     * @brief getSymbol Tries to retrieve the symbol for the given name.
     * @param fullPathName The full path to the symbol. Ex: sfsl.lang.int
     * @return A pointer to the #sfsl::sym::Symbol if found, otherwise nullptr.
     */
    Symbol* getSymbol(const std::string& fullPathName);

private:

    Scope* _scope;
};

}

}

#endif
