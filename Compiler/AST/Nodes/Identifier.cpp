//
//  Identifier.cpp
//  SFSL
//
//  Created by Romain Beguet on 17.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Identifier.h"
#include "../Visitors/ASTVisitor.h"

namespace sfsl {

namespace ast {

Identifier::Identifier(const std::string& name) : _name(name) {

}

Identifier::~Identifier() {

}

std::string Identifier::getName() {
    return _name;
}

SFSL_AST_ON_VISIT_CPP(Identifier)

}

}
