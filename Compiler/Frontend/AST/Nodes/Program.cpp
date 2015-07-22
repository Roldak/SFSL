//
//  Program.cpp
//  SFSL
//
//  Created by Romain Beguet on 16.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Program.h"
#include "../Visitors/ASTVisitor.h"

namespace sfsl {

namespace ast {

Program::Program(const std::vector<ModuleDecl*> &modules) : _modules(modules) {

}

const std::vector<ModuleDecl*>& Program::getModules() const {
    return _modules;
}

Program::~Program() {

}

SFSL_AST_ON_VISIT_CPP(Program)

}

}
