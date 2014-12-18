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
         * @brief Creates a BasePositionnable
         */
        Positionnable();

        /**
         * @brief Creates a BasePositionnable
         * @param pos its position
         * @param source its source name
         */
        Positionnable(size_t pos, std::string *source);

        virtual ~Positionnable();

        /**
         * @brief sets the position of the object to the given parameters
         * @param pos the new position
         * @param source the new file name
         */
        void setPos(size_t pos, std::string* source);

        /**
         * @brief sets the position of the object to be the same as the Positionnable given in parameter
         * @param other the Positionnable which contains the position to copy
         */
        void setPos(const Positionnable& other);

        /**
         * @return the position of the Positionnable
         */
        size_t getPosition() const;

        /**
         * @return the name of the source at the origin of the Positionnable
         */
        std::string* getSourceName() const;

    private:

        size_t _pos;
        std::string* _source;

    };

}

}

#endif
