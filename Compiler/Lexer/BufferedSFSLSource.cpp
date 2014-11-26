//
//  BufferedSFSLSource.cpp
//  SFSL
//
//  Created by Romain Beguet on 25.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "BufferedSFSLSource.h"

namespace sfsl {

namespace src {

BufferedSFSLSource::BufferedSFSLSource(SFSLSource &source, size_t maxBufferSize)
    : _source(source), _maxBufferSize(maxBufferSize), _buffer(new char[maxBufferSize]) {
    produceNext();
}

BufferedSFSLSource::~BufferedSFSLSource() {
    delete _buffer;
}

bool BufferedSFSLSource::hasNext() const {
    return _index < _buffSize;
}

char BufferedSFSLSource::getNext()  {
    char toRet = _buffer[_index++];

    if (_index >= _buffSize) {
        produceNext();
    }

    return toRet;
}

size_t BufferedSFSLSource::getPosition() const {
    return _source.getPosition();
}

std::string *BufferedSFSLSource::getSourceName() const {
    return _source.getSourceName();
}

common::Positionnable BufferedSFSLSource::currentPos() const {
    return _source.currentPos();
}

void BufferedSFSLSource::produceNext() {
    _index = 0;
    _buffSize = _source.getNexts(_buffer, _maxBufferSize);
}

}

}
