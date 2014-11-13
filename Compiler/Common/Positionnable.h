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

    /**
     * @brief An interface that represents an object that is bound
     * to a position (a position in its source and the name of this source)
     */
    class Positionnable {
    public:

        /**
         * @brief Creates a Positionnable
         */
        Positionnable();

        /**
         * @brief Creates a Positionnable
         * @param pos its position
         * @param source its source name
         */
        Positionnable(size_t pos, const std::string& source);

        template<typename T>
        /**
         * @brief sets the position of the object to the given parameters
         * @param pos the new position
         * @param source the new file name
         * @return this
         */
        T* setPos(size_t pos, const std::string& source) {
            _pos = pos;
            _source = source;
            return static_cast<T*>(this);
        }

        template<typename T>
        /**
         * @brief sets the position of the object to be the same as the Positionnable given in parameter
         * @param other the Positionnable which contains the position to copy
         * @return this
         */
        T* setPos(const Positionnable& other) {
            return setPos<T>(other._pos, other._source);
        }

        /**
         * @return the position of the Positionnable
         */
        size_t getPosition() const;

        /**
         * @return the name of the source at the origin of the Positionnable
         */
        const std::string& getSourceName() const;

    private:

        size_t _pos;
        std::string _source;

    };

}

}

#endif
