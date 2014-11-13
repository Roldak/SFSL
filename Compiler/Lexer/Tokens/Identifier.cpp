//
//  Identifier.cpp
//  SFSL
//
//  Created by Romain Beguet on 13.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Identifier.h"

namespace sfsl {

namespace tok {

Identifier::Identifier(const std::string &id)  : _id(id) {

}

Identifier::~Identifier() {

}

TOK_TYPE Identifier::getTokenType() {
    return TOK_ID;
}

std::string Identifier::toString() {
    return _id;
}

}

}
