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
#include "BufferedInputSource.h"
#include "Tokens/Token.h"
#include "../Common/CompilationContext.h"

namespace sfsl {

namespace lex {

    /**
     * @brief Transforms an SFSL source file given as an inputstream into a sequence of #sfsl::tok::Token
     * that are accessible with Lexer#getNext().
     */
    class Lexer {
    public:

        /**
         * @brief Creates a Lexer object
         * @param mngr The memory manager used throughout the tokenization process
         * @param source The input source
         * @param sourceBufferSize The size of the buffer used by the source reader
         */
        Lexer(CompCtx_Ptr& ctx, src::InputSource& source, size_t sourceBufferSize = 128);

        /**
         * @return True if there are more tokens to come, otherwise false
         */
        bool hasNext() const;

        /**
         * @return The next produced #sfsl::tok::Token
         */
        tok::Token* getNext();

    private:

        enum CHR_KIND { CHR_SYMBOL, CHR_CHARACTER, CHR_DIGIT, CHR_SPACE, CHR_QUOTE, CHR_EMPTY, CHR_UNKNOWN };
        enum STR_KIND { STR_SYMBOL, STR_ID, STR_STRING_LIT, STR_INT_LIT, STR_REAL_LIT, STR_UNKNOWN };

        struct CharInfo {
            char chr;
            size_t pos;
            CHR_KIND kind;
        };

        static bool isStillValid(STR_KIND strKind, const std::string& soFar, CHR_KIND chrKind, char nextChar);
        static bool isValidSymbol(const std::string& str);
        static bool isValidKeyword(const std::string& str);

        static CHR_KIND charKindFromChar(char c);
        static STR_KIND strKindFromCharKind(CHR_KIND c);

        CharInfo readCharInfo();

        void produceNext();

        tok::Token* buildToken(STR_KIND kind, const std::string& soFar) const;
        tok::Token* getRightTokenFromIdentifier(const std::string& str) const;

        void handleStringLitteral(std::string& soFar);
        bool tryHandleComments(const std::string& soFar, char next);
        void handleMultiLineComment();
        void handleSingleLineComment();

        CompCtx_Ptr _ctx;
        src::BufferedInputSource _source;

        tok::Token* _curToken;

        CharInfo _lastChar;

    };


}

}

#endif
