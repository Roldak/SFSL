//
//  Positionnable.cpp
//  SFSL
//
//  Created by Romain Beguet on 12.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Positionnable.h"
#include "../../Utils/Utils.h"

namespace sfsl {

namespace common {

Positionnable::Positionnable() {

}

Positionnable::Positionnable(size_t startPos, size_t endPos, src::InputSourceName source) {
    setPos(startPos, endPos, source);
}

Positionnable::~Positionnable() {

}

void Positionnable::setPos(size_t startPos, size_t endPos, src::InputSourceName source) {
    _startPos = startPos;
    _endPos = endPos;
    _source = source;
}

void Positionnable::setPos(const Positionnable& other) {
    setPos(other._startPos, other._endPos, other._source);
}

void Positionnable::setStartPos(size_t startPos) {
    _startPos = startPos;
}

void Positionnable::setEndPos(size_t endPos) {
    _endPos = endPos;
}

size_t Positionnable::getStartPosition() const{
    return _startPos;
}

size_t Positionnable::getEndPosition() const{
    return _endPos;
}

src::InputSourceName Positionnable::getSourceName() const {
    return _source;
}

std::string Positionnable::positionStr() const {
    return _source.getName() + ":" + utils::T_toString(_startPos) + ":" + utils::T_toString(_endPos);
}

}

}
