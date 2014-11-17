//
//  Identifier.h
//  SFSL
//
//  Created by Romain Beguet on 17.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__AST_Identifier__
#define __SFSL__AST_Identifier__

#include <iostream>
#include "ASTNode.h"

namespace sfsl {

namespace ast {

class Identifier : public ASTNode {
public:

    Identifier(const std::string& name);
    virtual ~Identifier();

    SFSL_AST_ON_VISIT_H

    std::string getName();

private:

    const std::string _name;

};

}

}

#endif
