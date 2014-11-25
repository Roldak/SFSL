//
//  BufferedSFSLSource.h
//  SFSL
//
//  Created by Romain Beguet on 25.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__BufferedSFSLSource__
#define __SFSL__BufferedSFSLSource__

#include <iostream>
#include "SFSLSource.h"

namespace sfsl {

namespace src {

template<size_t BUFFER_SIZE>
/**
 * @brief A SFSL Source overlay that uses a buffer of a constant size
 * to store the next characters of the input.
 */
class BufferedSFSLSource {
public:

    /**
     * @brief Creates a BufferedSFSLSource from the input source
     * @param source The input source from which to fetch the data
     */
    BufferedSFSLSource(SFSLSource& source) : _source(source) {
        produceNext();
    }

    ~BufferedSFSLSource() {}

    /**
     * @return True if it is still possible to fetch characeters from the input
     */
    bool hasNext() const {
        return _index < _buffSize;
    }

    /**
     * @return The next character from the input
     */
    char getNext() {
        char toRet = _buffer[_index++];

        if (_index >= _buffSize) {
            produceNext();
        }

        return toRet;
    }

    /**
     * @return The current position in the source
     */
    size_t getPosition() {
        return _source.getPosition();
    }

    /**
     * @return The name of the source
     */
    std::string* getSourceName() {
        return _source.getSourceName();
    }

    /**
     * @return A Positionnable corresponding to the current position
     */
    common::Positionnable currentPos() {
        return _source.currentPos();
    }

private:

    void produceNext() {
        _index = 0;
        _buffSize = _source.getNexts(_buffer, BUFFER_SIZE);
    }

    SFSLSource& _source;

    char _buffer[BUFFER_SIZE];
    size_t _index;
    size_t _buffSize;
};

}

}

#endif
