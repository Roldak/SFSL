//
//  Positionnable.h
//  SFSL
//
//  Created by Romain Beguet on 12.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Positionnable__
#define __SFSL__Positionnable__

#include <iostream>

namespace sfsl {

namespace common {

    template<typename T>
    class Positionnable {
    public:

        const T& setPos(size_t pos, const std::string& file) {
            _pos = pos;
            _file = file;
            return this;
        }

        const T& setPos(const Positionnable& other) {
            _pos = other._pos;
            _file = other._file;
            return this;
        }

    private:

        size_t _pos;
        std::string _file;

    };

}

}

#endif
