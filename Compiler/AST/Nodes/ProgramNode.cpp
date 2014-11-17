//
//  ProgramNode.cpp
//  SFSL
//
//  Created by Romain Beguet on 16.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "ProgramNode.h"
#include "../Visitors/ASTVisitor.h"

namespace sfsl {

namespace ast {

ProgramNode::ProgramNode(const std::vector<ModuleNode*> &modules) : _modules(modules) {

}

ProgramNode::~ProgramNode() {

}

SFSL_AST_ON_VISIT_CPP(ProgramNode)

}

}
