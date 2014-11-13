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
#include "SFSLSource.h"
#include "Tokens/Token.h"
#include "../Common/MemoryManager.h"

namespace sfsl {

namespace lex {

    /**
     * @brief Transforms an SFSL source file given as an inputstream into a sequence of #sfsl::tok::Token
     * that are accessible with Lexer#getNext()
     */
    class Lexer {
    public:

        /**
         * @brief Creates a Lexer object
         * @param mngr The memory manager used throughout the tokenization process
         * @param source The input source
         */
        Lexer(common::AbstractMemoryManager& mngr, src::SFSLSource& source);

        /**
         * @return True if there are more tokens to come, otherwise false
         */
        bool hasNext() const;

        /**
         * @return The next produced #sfsl::tok::Token
         */
        tok::Token* getNext();

    private:

        enum CHR_KIND { CHR_SYMBOL, CHR_CHARACTER, CHR_DIGIT, CHR_SPACE, CHR_QUOTE, CHR_UNKNOWN };
        enum STR_KIND { STR_SYMBOL, STR_ID, STR_STRING_LIT, STR_INT_LIT, STR_REAL_LIT, STR_UNKNOWN };

        void produceNext();

        void buildToken();
        bool isStillValid(STR_KIND strKind, const std::string& soFar, CHR_KIND chrKind, char nextChar);
        bool isValidSymbol(const std::string& str);
        CHR_KIND charKindFromChar(char c);
        STR_KIND strKindFromCharKind(CHR_KIND c);



        common::AbstractMemoryManager& _mngr;

        src::SFSLSource& _source;

        tok::Token* _curToken;

    };


}

}

#endif
