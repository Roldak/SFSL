//
//  Lexer.cpp
//  SFSL
//
//  Created by Romain Beguet on 12.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Lexer.h"

namespace sfsl {

using namespace tok;

namespace lex {

Lexer::Lexer(common::AbstractMemoryManager& mngr, std::istream& input) : _mngr(mngr), _input(input) {
    produceNext();
}

bool Lexer::hasNext() const {
    return _nextToken->getTokenType() != TOK_EOF;
}

Token* Lexer::getNext() {
    Token* current = _nextToken;
    produceNext();
    return current;
}

void Lexer::produceNext() {

    char c;

    if (_input >> c) {

    } else {

    }
}


}

}
