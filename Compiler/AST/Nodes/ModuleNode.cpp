//
//  ModuleNode.cpp
//  SFSL
//
//  Created by Romain Beguet on 16.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "ModuleNode.h"
#include "../Visitors/ASTVisitor.h"

namespace sfsl {

namespace ast {

ModuleNode::ModuleNode() {

}

ModuleNode::~ModuleNode() {

}

SFSL_AST_ON_VISIT_CPP(ModuleNode)

}

}
