//
//  Lexer.cpp
//  SFSL
//
//  Created by Romain Beguet on 12.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Lexer.h"

#include "Tokens/Keyword.h"
#include "Tokens/Operators.h"
#include "Tokens/Others.h"
#include "../../Utils/Utils.h"

namespace sfsl {

using namespace tok;

namespace lex {

Lexer::Lexer(common::AbstractMemoryManager& mngr, src::SFSLSource& source) :
    _mngr(mngr), _source(source) {

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

    char chr;
    size_t initPos;

    do {
        if (!_source.hasNext()) {
            _curToken = _mngr.New<EOFToken>()->setPos<Token>(_source.currentPos());
            return;
        }

        initPos = _source.getPosition();
        chr = _source.getNext();

    } while(chrutils::isWhiteSpace(chr));

    std::string soFar(chr, 1);

    CHR_KIND chrKind = charKindFromChar(chr);
    STR_KIND strKind = strKindFromCharKind(chrKind);

    while (_source.hasNext()) {

        chr = _source.getNext();
        chrKind = charKindFromChar(chr);



    }

}

void Lexer::buildToken() {

}

bool Lexer::isStillValid(STR_KIND strKind, const std::string& soFar, CHR_KIND chrKind, char nextChar) {
    switch (strKind) {
    case STR_SYMBOL:    return chrKind == CHR_SYMBOL && isValidSymbol(soFar + nextChar);
    }
}

bool Lexer::isValidSymbol(const std::string &str) {
    return Operator::OperTypeFromString(str) != OPER_UNKNOWN;
}

Lexer::CHR_KIND Lexer::charKindFromChar(char c) {
    if (chrutils::isSymbol(c))          return CHR_SYMBOL;
    else if (chrutils::isCharacter(c))  return CHR_CHARACTER;
    else if (chrutils::isNumeric(c))    return CHR_DIGIT;
    else if (chrutils::isWhiteSpace(c)) return CHR_SPACE;
    else if (chrutils::isQuote(c))      return CHR_QUOTE;
    else                                return CHR_UNKNOWN;
}

Lexer::STR_KIND Lexer::strKindFromCharKind(Lexer::CHR_KIND c) {
    switch (c) {
    case CHR_SYMBOL:    return STR_SYMBOL;
    case CHR_CHARACTER: return STR_ID;
    case CHR_DIGIT:     return STR_INT_LIT;
    case CHR_QUOTE:     return STR_STRING_LIT;
    default:            return STR_UNKNOWN;
    }
}

}

}
