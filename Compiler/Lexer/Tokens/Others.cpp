//
//  Others.cpp
//  SFSL
//
//  Created by Romain Beguet on 13.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Others.h"

namespace sfsl {

namespace tok {

// EOF

EOFToken::EOFToken() {

}

EOFToken::~EOFToken() {

}

TOK_TYPE EOFToken::getTokenType() const {
    return TOK_EOF;
}

std::string EOFToken::toString() const {
    return "EOF";
}

// BAD

BadToken::BadToken() {

}

BadToken::~BadToken() {

}

TOK_TYPE BadToken::getTokenType() const {
    return TOK_BAD;
}

std::string BadToken::toString() const {
    return "BAD";
}

}

}
