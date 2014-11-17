//
//  ASTVisitor.h
//  SFSL
//
//  Created by Romain Beguet on 16.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__ASTVisitor__
#define __SFSL__ASTVisitor__

#include <iostream>
#include "../Nodes/ASTNode.h"

namespace sfsl {

namespace ast {

/**
 * @brief An abstract class representing a visitor of ASTNodes.
 */
class ASTVisitor {
public:

    /**
     * @brief Should never be called.
     * @param node The visited node
     */
    virtual void visit(ASTNode* node);
};

}

}

#endif
