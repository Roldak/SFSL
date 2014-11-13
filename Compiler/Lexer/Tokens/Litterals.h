//
//  Litterals.h
//  SFSL
//
//  Created by Romain Beguet on 13.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Litterals__
#define __SFSL__Litterals__

#include "Token.h"

namespace sfsl {

namespace tok {

    class IntLitteral : public Token {
    public:

        virtual TOK_TYPE getTokenType();
        virtual std::string toString();

    private:

    };

    class RealLitteral : public Token {
    public:

        virtual TOK_TYPE getTokenType();
        virtual std::string toString();

    private:

    };

    class StringLitteral : public Token {
    public:

        virtual TOK_TYPE getTokenType();
        virtual std::string toString();

    private:

    };

}

}

#endif
