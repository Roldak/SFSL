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
#include "ASTNode.h"
#include "Declarations.h"
#include "../Symbols/Scoped.h"

namespace sfsl {

namespace ast {

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

}

}

#endif
