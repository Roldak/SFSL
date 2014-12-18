//
//  Positionnable.cpp
//  SFSL
//
//  Created by Romain Beguet on 12.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Positionnable.h"

namespace sfsl {

namespace common {

Positionnable::Positionnable() {

}

Positionnable::Positionnable(size_t pos, std::string *source) : _pos(pos), _source(source) {

}

Positionnable::~Positionnable() {

}

void Positionnable::setPos(size_t pos, std::string* source) {
    _pos = pos;
    _source = source;
}

void Positionnable::setPos(const Positionnable& other) {
    setPos(other._pos, other._source);
}

size_t Positionnable::getPosition() const{
    return _pos;
}

std::string* Positionnable::getSourceName() const {
    return _source;
}

}

}
