//
//  SFSLSource.cpp
//  SFSL
//
//  Created by Romain Beguet on 13.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "SFSLSource.h"

#include "../Common/ManageableWrapper.h"

namespace sfsl {

namespace src {

// SFSL SOURCE PATH

SFSLSourceName SFSLSourceName::make(const std::shared_ptr<common::CompilationContext>& compilationContext, const std::string& name) {
    return SFSLSourceName(compilationContext.get()->memoryManager().New<common::ManageableWrapper<std::string>>(name)->getRef());
}

SFSLSourceName::SFSLSourceName(std::string& name) : _name(name) {

}

std::string &SFSLSourceName::getName() const {
    return _name;
}

// SFSL SOURCE

SFSLSource::SFSLSource(const SFSLSourceName& sourceName) : _position(0), _sourceName(sourceName.getName()) {

}

size_t SFSLSource::getPosition() const {
    return _position - 1;
}

std::string* SFSLSource::getSourceName() const {
    return &_sourceName;
}

common::Positionnable SFSLSource::currentPos() const {
    return common::Positionnable(getPosition(), getPosition() + 1, getSourceName());
}

// INPUT STREAM SOURCE

SFSLInputStream::SFSLInputStream(const SFSLSourceName& sourceName, std::istream& input) : SFSLSource(sourceName), _input(input) {
    produceNext();
}

size_t SFSLInputStream::getNexts(char* buffer, size_t maxBufferSize) {
    size_t i = 0;

    while (_hasNext && i < maxBufferSize) {
        buffer[i++] = _curChar;
        produceNext();
    }

    return i;
}

const int SFSLInputStream::CHAR_EOF = std::char_traits<char>::eof();

void SFSLInputStream::produceNext() {
    ++_position;
    _curChar = _input.get();
    _hasNext = (_curChar != CHAR_EOF);
}

// INPUT STRING SOURCE

SFSLInputString::SFSLInputString(const SFSLSourceName& sourceName, const std::string &source)
    : SFSLSource(sourceName), _input(source), _size(source.size()), _curIndex(0) {

}

size_t SFSLInputString::getNexts(char* buffer, size_t maxBufferSize) {
    size_t i = 0;

    while (_curIndex < _size && i < maxBufferSize) {
        buffer[i++] = _input[_curIndex++];
    }

    return i;
}

}

}
