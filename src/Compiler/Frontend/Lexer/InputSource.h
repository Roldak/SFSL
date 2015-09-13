//
//  InputSource.h
//  SFSL
//
//  Created by Romain Beguet on 13.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__InputSource__
#define __SFSL__InputSource__

#include <iostream>
#include "InputSourceName.h"
#include "../../Common/Positionnable.h"
#include "../../Common/CompilationContext.h"

namespace sfsl {

namespace src {

/**
 * @brief Abstract class representing a source of code
 */
class InputSource {
public:

    InputSource(InputSourceName sourceName);

    virtual ~InputSource();

    /**
     * @brief Fills the buffer with maxBufferSize characters (or less, if the end of this input
     * was reached before getting to that number)
     *
     * @param buffer The buffer in which to fill the next characters
     * @param maxBufferSize The maximum number of character to fill the buffer with
     * @return The number of characters which the buffer could be filled with
     */
    virtual size_t getNexts(char* buffer, size_t maxBufferSize) = 0;

    /**
     * @return The current position in the source
     */
    size_t getPosition() const;

    /**
     * @return The name of the source
     */
    src::InputSourceName getSourceName() const;

    /**
     * @return A Positionnable corresponding to the current position
     */
    common::Positionnable currentPos() const;

protected:

    size_t _position;
    src::InputSourceName _sourceName;
};

/**
 * @brief An InputSource that uses an input stream as input for the source code
 */
class IStreamSource : public InputSource {
public:

    /**
     * @brief Creates an IStreamSource
     * @param sourceName the name of the source
     * @param input the std::istream input
     */
    IStreamSource(InputSourceName sourceName, std::istream& input);

    virtual ~IStreamSource();

    virtual size_t getNexts(char* buffer, size_t maxBufferSize) override;

private:

    static const int CHAR_EOF;

    void produceNext();

    std::istream& _input;

    bool _hasNext;
    char _curChar;

};

/**
 * @brief A StringSource that uses a string as input for the source code
 */
class StringSource : public InputSource {
public:

    /**
     * @brief Creates a StringSource
     * @param sourceName the path to the source
     * @param source the std::string input
     */
    StringSource(InputSourceName sourceName, const std::string& source);

    virtual ~StringSource();

    virtual size_t getNexts(char* buffer, size_t maxBufferSize) override;

private:

    const std::string _input;

    size_t _size;
    size_t _curIndex;

};

}

}

#endif
