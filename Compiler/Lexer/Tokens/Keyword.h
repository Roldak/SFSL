//
//  Keyword.h
//  SFSL
//
//  Created by Romain Beguet on 13.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Keyword__
#define __SFSL__Keyword__

#include "Token.h"

namespace sfsl {

namespace tok {

    class Keyword : public Token {
    public:

        virtual TOK_TYPE getTokenType();
        virtual std::string toString();

    private:

    };

}

}

#endif
