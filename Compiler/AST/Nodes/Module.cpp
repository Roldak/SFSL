//
//  Module.cpp
//  SFSL
//
//  Created by Romain Beguet on 16.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Module.h"
#include "../Visitors/ASTVisitor.h"

namespace sfsl {

namespace ast {

Module::Module(Identifier *name) : _name(name) {

}

Module::~Module() {

}

SFSL_AST_ON_VISIT_CPP(Module)

}

}
