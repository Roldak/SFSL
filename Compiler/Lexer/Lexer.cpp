//
//  Lexer.cpp
//  SFSL
//
//  Created by Romain Beguet on 12.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Lexer.h"

#include "Tokens/Identifier.h"
#include "Tokens/Keyword.h"
#include "Tokens/Operators.h"
#include "Tokens/Litterals.h"
#include "Tokens/Others.h"
#include "../../Utils/Utils.h"

namespace sfsl {

using namespace tok;

namespace lex {

Lexer::Lexer(common::AbstractMemoryManager& mngr, src::SFSLSource& source) :
    _mngr(mngr), _source(source) {

    _lastChar.kind = CHR_EMPTY;
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

    while (_lastChar.kind == CHR_EMPTY || _lastChar.kind == CHR_SPACE) {
        if (!_source.hasNext()) {
            _curToken = _mngr.New<EOFToken>()->setPos<Token>(_source.currentPos());
            return;
        }

        _lastChar = readCharInfo();
    }

    size_t initPos = _lastChar.pos;

    std::string soFar;
    soFar += _lastChar.chr;

    STR_KIND strKind = strKindFromCharKind(_lastChar.kind);

    while (_source.hasNext()) {

        _lastChar = readCharInfo();

        if (tryHandleComments(soFar + _lastChar.chr)){
            _lastChar.kind = CHR_EMPTY;
            produceNext();
            return;
        }
        else if (isStillValid(strKind, soFar, _lastChar.kind, _lastChar.chr)) {
            soFar += _lastChar.chr;
        }
        else if (strKind == STR_INT_LIT && _lastChar.chr == '.') {
            strKind = STR_REAL_LIT;
            soFar += _lastChar.chr;
        }
        else if (strKind == STR_UNKNOWN) {
            // ERROR
            _curToken = _mngr.New<BadToken>(soFar)->setPos<Token>(initPos, _source.getSourceName());
            return;
        }
        else {
            if (strKind == STR_STRING_LIT) {
                handleStringLitteral(soFar);
                _lastChar.kind = CHR_EMPTY;
            }
            _curToken = buildToken(strKind, soFar)->setPos<Token>(initPos, _source.getSourceName());
            return;
        }

    }

    _lastChar.kind = CHR_EMPTY;
    _curToken = buildToken(strKind, soFar)->setPos<Token>(initPos, _source.getSourceName());
}

Lexer::CharInfo Lexer::readCharInfo() {
    size_t pos = _source.getPosition();
    char chr = _source.getNext();
    CHR_KIND kind = charKindFromChar(chr);

    return CharInfo{
        .chr = chr,
        .pos = pos,
        .kind = kind
    };
}

Token* Lexer::buildToken(STR_KIND kind, const std::string &soFar) const {
    switch (kind) {
    case STR_SYMBOL:        return _mngr.New<Operator>(Operator::OperTypeFromString(soFar));
    case STR_ID:            return getRightTokenFromIdentifier(soFar);
    case STR_INT_LIT:       return _mngr.New<IntLitteral>(utils::String_toT<sfsl_int_t>(soFar));
    case STR_REAL_LIT:      return _mngr.New<RealLitteral>(utils::String_toT<sfsl_real_t>(soFar));
    case STR_STRING_LIT:    return _mngr.New<StringLitteral>(soFar);
    default:                return _mngr.New<BadToken>(soFar);
    }
}

Token* Lexer::getRightTokenFromIdentifier(const std::string &str) const{
    if (isValidKeyword(str)) {
        return _mngr.New<Keyword>(Keyword::KeywordTypeFromString(str));
    } else if (isValidSymbol(str)) {
        return _mngr.New<Operator>(Operator::OperTypeFromString(str));
    } else {
        return _mngr.New<Identifier>(str);
    }
}

void Lexer::handleStringLitteral(std::string &soFar) {
    soFar = "";
    soFar += _lastChar.chr;

    bool escaping = false;

    for(;;) {
        if (!_source.hasNext()) {
            // ERROR
            exit(1);
        }

        char c = _source.getNext();

        if (escaping) {
            escaping = false;
            if (chrutils::escapedChar(c)) {
                soFar += c;
            } else {
                // ERROR
                exit(1);
            }
            continue;
        }

        if (c == '\"') {
            return;
        } else if (c == '\\') {
            escaping = true;
        } else if (c != '\n' && c != '\r') {
            soFar += c;
        }
    }
}

bool Lexer::tryHandleComments(const std::string &soFar) {
    if (soFar == "//")
        handleSingleLineComment();
    else if (soFar == "/*")
        handleMultiLineComment();
    else
        return false;

    return true;
}

void Lexer::handleMultiLineComment() {
    char oldChr = 0;
    char chr = 0;

    while (oldChr != '*' && chr != '/') {
        if (!_source.hasNext()) {
            // ERROR
            exit(1);
        }

        oldChr = chr;
        chr = _source.getNext();
    }
}

void Lexer::handleSingleLineComment() {
    char chr = 0;
    while (_source.hasNext() && chr != '\n' && chr != '\r') {
        chr = _source.getNext();
    }
}

bool Lexer::isStillValid(STR_KIND strKind, const std::string& soFar, CHR_KIND chrKind, char nextChar) {
    switch (strKind) {
    case STR_SYMBOL:    return chrKind == CHR_SYMBOL && isValidSymbol(soFar + nextChar);
    case STR_ID:        return chrKind == CHR_CHARACTER || chrKind == CHR_DIGIT;
    case STR_INT_LIT:   return chrKind == CHR_DIGIT;
    case STR_REAL_LIT:  return chrKind == CHR_DIGIT;
    case STR_STRING_LIT:return false;
    default:            return false;
    }
}

bool Lexer::isValidSymbol(const std::string &str) {
    return Operator::OperTypeFromString(str) != OPER_UNKNOWN;
}

bool Lexer::isValidKeyword(const std::string &str) {
    return Keyword::KeywordTypeFromString(str) != KW_UNKNOWN;
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
