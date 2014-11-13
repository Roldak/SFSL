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

namespace sfsl {

namespace src {

    class SFSLSource {
    public:

        SFSLSource(const std::string& sourceName);

        virtual bool hasNext() const = 0;
        virtual char getNext() = 0;

        std::string getSourceName();

    private:

        const std::string _sourceName;
    };

    class SFSLInputStream : public SFSLSource {
    public:

        SFSLInputStream(const std::string& sourceName, std::istream& input);

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
