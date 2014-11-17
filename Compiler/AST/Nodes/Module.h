//
//  Module.h
//  SFSL
//
//  Created by Romain Beguet on 16.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Module__
#define __SFSL__Module__

#include <iostream>
#include "Identifier.h"

namespace sfsl {

namespace ast {

class Module : public ASTNode {
public:

    Module(Identifier* name);
    virtual ~Module();

    SFSL_AST_ON_VISIT_H

private:

    Identifier* _name;

};

}

}

#endif
