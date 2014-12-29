//
//  TypeNodes.cpp
//  SFSL
//
//  Created by Romain Beguet on 23.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "TypeNodes.h"
#include "../Visitors/ASTVisitor.h"

namespace sfsl {

namespace ast {

// TYPED

Typed::~Typed() {

}

void Typed::setType(type::Type *type) {
    _type = type;
}

type::Type* Typed::type() {
    return _type;
}

}

}
