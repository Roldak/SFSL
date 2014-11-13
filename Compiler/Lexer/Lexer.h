//
//  Lexer.h
//  SFSL
//
//  Created by Romain Beguet on 12.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Lexer__
#define __SFSL__Lexer__

#include <iostream>
#include "Tokens/Token.h"
#include "../Common/MemoryManager.h"

namespace sfsl {

namespace lex {

    /**
     * @brief The Lexer class
     * Transforms an SFSL source file given as an inputstream into a sequence of #sfsl::tok::Token
     * that are accessible with Lexer#getNext()
     */
    class Lexer {
    public:

        /**
         * @brief creates a Lexer object
         * @param mngr The memory manager used throughout the tokenization process
         * @param input The input source
         */
        Lexer(const common::AbstractMemoryManager& mngr, const std::istream& input);

        /**
         * @return True if there are more tokens to come, otherwise false
         */
        bool hasNext();

        /**
         * @return The next produced #sfsl::tok::Token
         */
        tok::Token* getNext();

    private:

        const common::AbstractMemoryManager& _mngr;
        const std::istream& _input;

    };


}

}

#endif
