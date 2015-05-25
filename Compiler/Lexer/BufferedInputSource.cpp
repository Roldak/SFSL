//
//  BufferedInputSource.cpp
//  SFSL
//
//  Created by Romain Beguet on 25.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "BufferedInputSource.h"

namespace sfsl {

namespace src {

BufferedInputSource::BufferedInputSource(InputSource &source, size_t maxBufferSize)
    : _source(source), _maxBufferSize(maxBufferSize), _buffer(new char[maxBufferSize]) {
    produceNext();
}

BufferedInputSource::~BufferedInputSource() {
    delete _buffer;
}

bool BufferedInputSource::hasNext() const {
    return _index < _buffSize;
}

char BufferedInputSource::getNext()  {
    char toRet = _buffer[_index++];

    if (_index >= _buffSize) {
        produceNext();
    }

    return toRet;
}

size_t BufferedInputSource::getPosition() const {
    return _source.getPosition() - _buffSize + _index;
}

InputSourceName BufferedInputSource::getSourceName() const {
    return _source.getSourceName();
}

common::Positionnable BufferedInputSource::currentPos() const {
    return _source.currentPos();
}

void BufferedInputSource::produceNext() {
    _index = 0;
    _buffSize = _source.getNexts(_buffer, _maxBufferSize);
}

}

}
