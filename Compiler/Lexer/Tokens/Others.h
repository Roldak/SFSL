//
//  Others.h
//  SFSL
//
//  Created by Romain Beguet on 13.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Others__
#define __SFSL__Others__

#include "Token.h"

namespace sfsl {

namespace tok {

    /**
     * @brief Represents the last token of the source
     */
    class EOFToken : public Token {
    public:

        /**
         * @brief Creates an EOF Token
         */
        EOFToken();
        virtual ~EOFToken();

        virtual TOK_TYPE getTokenType() const;
        virtual std::string toString() const;

    };

    /**
     * @brief Represents an invalid token
     */
    class BadToken : public Token {
    public:

        /**
         * @brief Creates a BAD Token
         */
        BadToken(const std::string& str);

        virtual ~BadToken();

        virtual TOK_TYPE getTokenType() const;
        virtual std::string toString() const;

    private:

        const std::string _str;
    };

}

}

#endif
