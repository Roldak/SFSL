//
//  Program.h
//  SFSL
//
//  Created by Romain Beguet on 16.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Program__
#define __SFSL__Program__

#include <iostream>
#include <vector>
#include "Expressions.h"
#include "TypeExpressions.h"
#include "KindExpressions.h"

#include "../../Symbols/Symbols.h"
#include "../../Symbols/Symbolic.h"

#include "../Utils/CanUseModules.h"
#include "../Utils/Annotations.h"

#include "../../Symbols/Scoped.h"

namespace sfsl {

namespace ast {

class ModuleDecl;

/**
 * @brief Represents the root of the program.
 * Contains all the modules declared in the global scope
 */
class Program : public ASTNode, public sym::Scoped {
public:

    Program(const std::vector<ModuleDecl*>& modules);
    virtual ~Program();

    SFSL_AST_ON_VISIT_H

    /**
     * @return All the modules declared in the program scope
     */
    const std::vector<ModuleDecl*>& getModules() const;

private:

    std::vector<ModuleDecl*> _modules;
};

/**
 * @brief The Module AST
 * Contains :
 *  - This module's name
 *  - The list of its submodules
 *  - The list of its classes
 *  - The list of its definitions
 */
class ModuleDecl :
        public ASTNode,
        public sym::Symbolic<sym::ModuleSymbol>,
        public CanUseModules,
        public Annotable {
public:

    ModuleDecl(Identifier* name,
               const std::vector<ModuleDecl*>& mods,
               const std::vector<TypeDecl*>& classes,
               const std::vector<DefineDecl*>& decls);
    virtual ~ModuleDecl();

    SFSL_AST_ON_VISIT_H

    /**
     * @return All the sub modules declarations that were made inside this module
     */
    const std::vector<ModuleDecl*>& getSubModules() const;

    /**
     * @return All the type definitions that were made inside this module
     */
    const std::vector<TypeDecl*>& getTypes() const;

    /**
     * @return All the declarations that were made inside this module
     */
    const std::vector<DefineDecl*>& getDeclarations() const;

    /**
     * @return The name of this module
     */
    Identifier* getName() const;

private:

    Identifier* _name;
    std::vector<ModuleDecl*> _mods;
    std::vector<TypeDecl*> _types;
    std::vector<DefineDecl*> _decls;
};

}

}

#endif
