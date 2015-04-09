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
#include "../Common/CompilationContext.h"

namespace sfsl {

namespace src {

    /**
     * @brief Simple class containing the name of the SFSLSource, which can be automatically managed
     */
    class SFSLSourceName final {
    public:

        friend class SFSLSource;

        /**
         * @brief Creates a new SFSLsourceName object and make it managed automatically
         * @param compilationContext The Compilation context from which to instantiate the object
         * @param name The name to the source file
         * @return The newly created SFSLSourceName
         */
        static SFSLSourceName make(const std::shared_ptr<common::CompilationContext>& compilationContext, const std::string& name);

    private:

        SFSLSourceName(std::string &name);
        std::string& getName() const;

        std::string& _name;
    };

    /**
     * @brief Abstract class representing a source of SFSL data
     */
    class SFSLSource {
    public:

        SFSLSource(const SFSLSourceName& sourceName);

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
        std::string* getSourceName() const;

        /**
         * @return A Positionnable corresponding to the current position
         */
        common::Positionnable currentPos() const;

    protected:

        size_t _position;
        std::string& _sourceName;
    };

    /**
     * @brief A SFSLSource that uses an input stream as input for the SFSL datas
     */
    class SFSLInputStream : public SFSLSource {
    public:

        /**
         * @brief Creates an SFSLInputStream
         * @param sourceName the name of the source
         * @param input the std::istream input
         */
        SFSLInputStream(const SFSLSourceName& sourceName, std::istream& input);

        virtual size_t getNexts(char* buffer, size_t maxBufferSize);

    private:

        static const int CHAR_EOF;

        void produceNext();

        std::istream& _input;

        bool _hasNext;
        char _curChar;

    };

    /**
     * @brief A SFSLSource that uses a string as input for the SFSL datas
     */
    class SFSLInputString : public SFSLSource {
    public:

        /**
         * @brief Creates a SFSLInputString
         * @param sourceName the path to the source
         * @param source the std::string input
         */
        SFSLInputString(const SFSLSourceName& sourceName, const std::string& source);

        virtual size_t getNexts(char* buffer, size_t maxBufferSize);

    private:

        const std::string _input;

        size_t _size;
        size_t _curIndex;

    };

}

}

#endif
