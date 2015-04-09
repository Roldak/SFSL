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
    BufferedSFSLSource(SFSLSource& source, size_t maxBufferSize);

    ~BufferedSFSLSource();

    /**
     * @return True if it is still possible to fetch characeters from the input
     */
    bool hasNext() const;

    /**
     * @return The next character from the input
     */
    char getNext();

    /**
     * @return The current position in the source
     */
    size_t getPosition() const;

    /**
     * @return The name of the source
     */
    SFSLSourceName getSourceName() const;

    /**
     * @return A Positionnable corresponding to the current position
     */
    common::Positionnable currentPos() const;

private:

    void produceNext();

    SFSLSource& _source;

    size_t _maxBufferSize;
    char* _buffer;

    size_t _index;
    size_t _buffSize;
};

}

}

#endif
