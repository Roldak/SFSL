//
//  EOF.cpp
//  SFSL
//
//  Created by Romain Beguet on 13.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "EOF.h"

namespace sfsl {

namespace tok {

EOFToken::EOFToken() {

}

EOFToken::~EOFToken() {

}

TOK_TYPE EOFToken::getTokenType() {
    return TOK_EOF;
}

std::string EOFToken::toString() {
    return "EOF";
}

}

}
