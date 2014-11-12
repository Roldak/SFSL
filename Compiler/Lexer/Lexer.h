//
//  Lexer.h
//  SFSL
//
//  Created by Romain Beguet on 12.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Lexer__
#define __SFSL__Lexer__

#include "Tokens/Token.h"

namespace sfsl {

namespace lex {

    class Lexer {
    public:

        Lexer(const std::string& input);

        bool hasMore();
        tok::Token* getNext();

    private:

        const std::string& _input;

    };


}

}

#endif
