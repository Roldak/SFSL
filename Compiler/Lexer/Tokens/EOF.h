//
//  EOF.h
//  SFSL
//
//  Created by Romain Beguet on 13.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__EOF__
#define __SFSL__EOF__

#include "Token.h"

namespace sfsl {

namespace tok {

    class EOFToken : public Token {
    public:

        EOFToken();
        virtual ~EOFToken();

        virtual TOK_TYPE getTokenType();
        virtual std::string toString();

    };

}

}

#endif
