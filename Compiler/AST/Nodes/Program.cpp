//
//  ProgramNode.cpp
//  SFSL
//
//  Created by Romain Beguet on 16.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Program.h"
#include "../Visitors/ASTVisitor.h"

namespace sfsl {

namespace ast {

Program::Program(const std::vector<Module*> &modules) : _modules(modules) {

}

Program::~Program() {

}

SFSL_AST_ON_VISIT_CPP(Program)

}

}
