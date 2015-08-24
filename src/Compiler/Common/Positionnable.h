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
#include "../Frontend/Lexer/InputSourceName.h"

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
         * @param pos Its position
         * @param source Its source name
         */
        Positionnable(size_t startPos, size_t endPos, src::InputSourceName source);

        virtual ~Positionnable();

        /**
         * @brief Sets the position of the object to the given parameters
         * @param pos The new position
         * @param source The new file name
         */
        void setPos(size_t startPos, size_t endPos, src::InputSourceName source);

        /**
         * @brief Sets the position of the object to be the same as the Positionnable given in parameter
         * @param other The Positionnable which contains the position to copy
         */
        void setPos(const Positionnable& other);

        /**
         * @brief Sets the start position
         * @param startPos The new position
         */
        void setStartPos(size_t startPos);

        /**
         * @brief Sets the end position
         * @param startPos The new position
         */
        void setEndPos(size_t endPos);

        /**
         * @return The start position of the Positionnable
         */
        size_t getStartPosition() const;

        /**
         * @return The end position of the Positionnable
         */
        size_t getEndPosition() const;

        /**
         * @return The name of the source at the origin of the Positionnable
         */
        src::InputSourceName getSourceName() const;

        /**
         * @return A readable representation of the position
         */
        std::string positionStr() const;

    private:

        size_t _startPos;
        size_t _endPos;
        src::InputSourceName _source;
    };

}

}

#endif
