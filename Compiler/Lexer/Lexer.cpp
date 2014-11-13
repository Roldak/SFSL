//
//  Lexer.cpp
//  SFSL
//
//  Created by Romain Beguet on 12.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Lexer.h"

namespace sfsl {

namespace lex {

Lexer::Lexer(const common::AbstractMemoryManager& mngr, const std::string &input) : _mngr(mngr), _input(input) {

}

bool Lexer::hasNext() {

}

tok::Token *Lexer::getNext() {

}


}

}
