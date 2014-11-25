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
class BufferedSFSLSource {
public:

    BufferedSFSLSource(SFSLSource& source);
    ~BufferedSFSLSource();

    /**
     * @return True if it is still possible to fetch characeters from the input
     */
    bool hasNext() const = 0;

    /**
     * @return The next character from the input
     */
    char getNext() = 0;

private:

    SFSLSource& _source;

    char _buffer[BUFFER_SIZE];
};

}

}

#endif
