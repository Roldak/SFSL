//
//  ProgramNode.h
//  SFSL
//
//  Created by Romain Beguet on 16.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__ProgramNode__
#define __SFSL__ProgramNode__

#include <iostream>
#include <vector>
#include "ASTNode.h"
#include "ModuleNode.h"

namespace sfsl {

namespace ast {

class ProgramNode : public ASTNode {
public:

    ProgramNode(const std::vector<ModuleNode*>& modules);
    virtual ~ProgramNode();

    SFSL_AST_ON_VISIT_H

private:

    std::vector<ModuleNode*> _modules;
};

}

}

#endif
