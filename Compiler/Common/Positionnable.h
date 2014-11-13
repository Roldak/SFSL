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
    /**
     * @brief An interface that represents an object that is bound
     * to a position (a position in a file and the name of this file)
     */
    class Positionnable {
    public:

        /**
         * @brief sets the position of the object to the given parameters
         * @param pos the new position
         * @param file the new file name
         * @return this
         */
        const T& setPos(size_t pos, const std::string& file) {
            _pos = pos;
            _file = file;
            return this;
        }

        /**
         * @brief sets the position of the object to be the same as the Positionnable given in parameter
         * @param other the Positionnable which contains the position to copy
         * @return this
         */
        const T& setPos(const Positionnable& other) {
            return setPos(other._pos, other._file);
        }

        /**
         * @return the position of the Positionnable
         */
        size_t getPosition() {
            return _pos;
        }

        /**
         * @return the name of the file at the origin of the Positionnable
         */
        std::string getFileName() {
            return _file;
        }

    private:

        size_t _pos;
        std::string _file;

    };

}

}

#endif
