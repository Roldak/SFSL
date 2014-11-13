//
//  SFSLSource.cpp
//  SFSL
//
//  Created by Romain Beguet on 13.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "SFSLSource.h"

namespace sfsl {

namespace src {

SFSLSource::SFSLSource(const std::string &sourceName) : _position(0), _sourceName(sourceName) {

}

size_t SFSLSource::getPosition() {
    return _position;
}

const std::string& SFSLSource::getSourceName() {
    return _sourceName;
}

common::Positionnable SFSLSource::currentPos() {
    return *common::Positionnable().setPos<common::Positionnable>(_position, _sourceName);
}

// INPUT STREAM SOURCE

SFSLInputStream::SFSLInputStream(const std::string &sourceName, std::istream &input) : SFSLSource(sourceName), _input(input) {
    produceNext();
}

bool SFSLInputStream::hasNext() const {
    return _hasNext;
}

char SFSLInputStream::getNext() {
    char next = _curChar;
    produceNext();
    return next;
}

void SFSLInputStream::produceNext() {
    ++_position;
    _hasNext = (_input >> _curChar);
}

}

}
