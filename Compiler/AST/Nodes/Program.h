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
#include "Module.h"

namespace sfsl {

namespace ast {

class Program : public ASTNode {
public:

    Program(const std::vector<Module*>& modules);
    virtual ~Program();

    SFSL_AST_ON_VISIT_H

private:

    std::vector<Module*> _modules;
};

}

}

#endif
