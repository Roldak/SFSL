//
//  InputSource.cpp
//  SFSL
//
//  Created by Romain Beguet on 13.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "InputSource.h"

namespace sfsl {

namespace src {

// SFSL SOURCE

InputSource::InputSource(InputSourceName sourceName) : _position(0), _sourceName(sourceName) {

}

InputSource::~InputSource() {

}

size_t InputSource::getPosition() const {
    return _position;
}

InputSourceName InputSource::getSourceName() const {
    return _sourceName;
}

common::Positionnable InputSource::currentPos() const {
    return common::Positionnable(getPosition(), getPosition() + 1, getSourceName());
}

// INPUT STREAM SOURCE

IStreamSource::IStreamSource(InputSourceName sourceName, std::istream& input) : InputSource(sourceName), _input(input) {
    produceNext();
}

IStreamSource::~IStreamSource() {

}

size_t IStreamSource::getNexts(char* buffer, size_t maxBufferSize) {
    size_t i = 0;

    while (_hasNext && i < maxBufferSize) {
        buffer[i++] = _curChar;
        produceNext();
    }

    return i;
}

const int IStreamSource::CHAR_EOF = std::char_traits<char>::eof();

void IStreamSource::produceNext() {
    ++_position;
    _curChar = _input.get();
    _hasNext = (_curChar != CHAR_EOF);
}

// INPUT STRING SOURCE

StringSource::StringSource(InputSourceName sourceName, const std::string &source)
    : InputSource(sourceName), _input(source), _size(source.size()) {

}

StringSource::~StringSource() {

}

size_t StringSource::getNexts(char* buffer, size_t maxBufferSize) {
    size_t i = 0;

    while (_position < _size && i < maxBufferSize) {
        buffer[i++] = _input[_position++];
    }

    return i;
}

}

}
