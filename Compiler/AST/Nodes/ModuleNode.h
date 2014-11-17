//
//  ModuleNode.h
//  SFSL
//
//  Created by Romain Beguet on 16.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__ModuleNode__
#define __SFSL__ModuleNode__

#include <iostream>
#include "ASTNode.h"

namespace sfsl {

namespace ast {

class ModuleNode : public ASTNode {
public:

    ModuleNode();
    virtual ~ModuleNode();

    SFSL_AST_ON_VISIT_H

private:
};

}

}

#endif
