//
//  Operators.h
//  SFSL
//
//  Created by Romain Beguet on 13.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Operators__
#define __SFSL__Operators__

#include "Token.h"

namespace sfsl {

namespace tok {

    /**
     * @brief Enumerates every possible Operator type
     */
    enum OPER_TYPE { };

    /**
     * @brief Represents an Operator Token (e.g. '+', '/', ';', '{')
     */
    class Operator : public Token {
    public:

        /**
         * @brief Creates an Operator Token
         * @param opType the type of the Operator
         */
        Operator(OPER_TYPE opType);
        virtual ~Operator();

        virtual TOK_TYPE getTokenType();
        virtual std::string toString();

    private:

        const OPER_TYPE _opType;

    };

}

}

#endif
