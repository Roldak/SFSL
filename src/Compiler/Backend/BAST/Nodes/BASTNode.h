//
//  BASTNode.h
//  SFSL
//
//  Created by Romain Beguet on 28.03.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__BASTNode__
#define __SFSL__BASTNode__

#define SFSL_BAST_ON_VISIT_H virtual void onVisit(BASTVisitor* visitor) override;
#define SFSL_BAST_ON_VISIT_CPP(clss) void clss::onVisit(BASTVisitor* visitor) { visitor->visit(this); }

#include <iostream>
#include "../../../Common/MemoryManageable.h"

namespace sfsl {

namespace bast {

class BASTVisitor;

/**
 * @brief An abstract class that represents a node of the Backend Abstract Syntax Tree.
 * This class must be extended by every Backend AST node.
 */
class BASTNode : public common::MemoryManageable {
public:

    /**
     * @brief Destroys the ASTNode
     */
    virtual ~BASTNode();

    /**
     * @brief Visits the ASTNode with the desired #sfsl::ast::ASTVisitor
     */
    virtual void onVisit(BASTVisitor* visitor) = 0;

private:

};

}

}

#endif
