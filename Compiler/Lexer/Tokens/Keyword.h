//
//  Token.h
//  SFSL
//
//  Created by Romain Beguet on 12.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Token__
#define __SFSL__Token__

#include "../../Common/MemoryManageable.h"
#include "../../Common/Positionnable.h"

namespace sfsl {

namespace tok {

    /**
     * @brief Enumerates every possible token type
     */
    enum TOK_TYPE { TOK_OPER, TOK_ID, TOK_KW, TOK_INT_LIT, TOK_REAL_LIT, TOK_EOF };

    /**
     * @brief Represents an abstract Token
     */
    class Token : public common::MemoryManageable, public common::Positionnable<Token> {
    public:

        /**
         * @return the type (#TOK_TYPE) of the token
         */
        virtual TOK_TYPE getTokenType() const = 0;

        /**
         * @return a string representation of the token
         */
        virtual std::string toString() const = 0;

    private:

    };


}

}

#endif
