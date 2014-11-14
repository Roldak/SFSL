//
//  SFSLSource.h
//  SFSL
//
//  Created by Romain Beguet on 13.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__SFSLSource__
#define __SFSL__SFSLSource__

#include <iostream>
#include "../Common/Positionnable.h"

namespace sfsl {

namespace src {

    /**
     * @brief Represents an abstract source of SFSL data
     */
    class SFSLSource {
    public:

        SFSLSource(std::string* sourceName);

        /**
         * @return True if it is still possible to fetch characeters from the input
         */
        virtual bool hasNext() const = 0;

        /**
         * @return The next character from the input
         */
        virtual char getNext() = 0;

        /**
         * @return The current position in the source
         */
        size_t getPosition();

        /**
         * @return The name of the source
         */
        std::string *getSourceName();

        /**
         * @return A Positionnable corresponding to the current position
         */
        common::Positionnable currentPos();

    protected:

        size_t _position;
        std::string* _sourceName;
    };

    /**
     * @brief A SFSLSource that uses an input stream as input for the SFSL datas
     */
    class SFSLInputStream : public SFSLSource {
    public:

        /**
         * @brief Creates an SFSLInputStream
         * @param sourceName the name of the source
         * @param input
         */
        SFSLInputStream(std::string* sourceName, std::istream& input);

        virtual bool hasNext() const;
        virtual char getNext();

    private:

        void produceNext();

        std::istream& _input;

        bool _hasNext;
        char _curChar;

    };

}

}

#endif
