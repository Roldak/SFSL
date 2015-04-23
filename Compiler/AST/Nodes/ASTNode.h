//
//  ASTNode.h
//  SFSL
//
//  Created by Romain Beguet on 16.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__ASTNode__
#define __SFSL__ASTNode__

#define SFSL_AST_ON_VISIT_H virtual void onVisit(ASTVisitor* visitor) override;
#define SFSL_AST_ON_VISIT_CPP(clss) void clss::onVisit(ASTVisitor* visitor) { visitor->visit(this); }

#include <iostream>
#include "../../Common/MemoryManageable.h"
#include "../../Common/Positionnable.h"

namespace sfsl {

namespace ast {

class ASTVisitor;

/**
 * @brief An abstract class that represents a node of the Abstract Syntax Tree.
 * This class must be extended by every AST node.
 */
class ASTNode : public common::Positionnable, public common::MemoryManageable {
public:

    /**
     * @brief Destroys the ASTNode
     */
    virtual ~ASTNode();

    /**
     * @brief Visits the ASTNode with the desired #sfsl::ast::ASTVisitor
     */
    virtual void onVisit(ASTVisitor* visitor) = 0;

private:

};

}

}

#endif
