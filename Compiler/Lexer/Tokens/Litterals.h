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

        IntLitteral(long long value);
        virtual ~IntLitteral();

        virtual TOK_TYPE getTokenType();
        virtual std::string toString();

    private:

        const long long _value;

    };

    class RealLitteral : public Token {
    public:

        RealLitteral(double value);
        virtual ~RealLitteral();

        virtual TOK_TYPE getTokenType();
        virtual std::string toString();

    private:

        const double _value;

    };

    class StringLitteral : public Token {
    public:

        StringLitteral(const std::string& value);
        virtual ~StringLitteral();

        virtual TOK_TYPE getTokenType();
        virtual std::string toString();

    private:

        const std::string _value;

    };

}

}

#endif
