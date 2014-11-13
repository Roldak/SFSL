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
#include "../../../Utils/Utils.h"

namespace sfsl {

namespace tok {

    class IntLitteral : public Token {
    public:

        IntLitteral(sfsl_int_t value);
        virtual ~IntLitteral();

        virtual TOK_TYPE getTokenType();
        virtual std::string toString();

    private:

        const sfsl_int_t _value;

    };

    class RealLitteral : public Token {
    public:

        RealLitteral(sfsl_real_t value);
        virtual ~RealLitteral();

        virtual TOK_TYPE getTokenType();
        virtual std::string toString();

    private:

        const sfsl_real_t _value;

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
