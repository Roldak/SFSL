//
//  Lexer.cpp
//  SFSL
//
//  Created by Romain Beguet on 12.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Lexer.h"

#include "Tokens/Keyword.h"
#include "Tokens/Others.h"

namespace sfsl {

using namespace tok;

namespace lex {

Lexer::Lexer(common::AbstractMemoryManager& mngr, src::SFSLSource& source) :
    _mngr(mngr), _source(source), _sourceName(source.getSourceName()), _curPos(0) {

    produceNext();
}

bool Lexer::hasNext() const {
    return _curToken->getTokenType() != TOK_EOF;
}

Token* Lexer::getNext() {
    Token* current = _curToken;
    produceNext();
    return current;
}

void Lexer::produceNext() {

    if (_source.hasNext()) {

        size_t pos = _curPos++;

        char c = _source.getNext();

        if (c == 'm') {
            _curToken = _mngr.New<Keyword>(Keyword::KeywordTypeFromString("module"))->setPos(pos, _sourceName);
        } else {
            _curToken = _mngr.New<BadToken>()->setPos(pos, _sourceName);
        }

    } else {
        _curToken = _mngr.New<EOFToken>()->setPos(_curPos, _sourceName);
    }

}

}

}
