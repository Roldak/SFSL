//
//  Operators.h
//  SFSL
//
//  Created by Romain Beguet on 13.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Operators__
#define __SFSL__Operators__

#include <unordered_map>
#include "Token.h"

namespace sfsl {

namespace tok {

    /**
     * @brief Enumerates every possible Operator type
     */
    enum OPER_TYPE { OPER_PLUS, OPER_UNKNOWN };

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

        virtual TOK_TYPE getTokenType() const;
        virtual std::string toString() const;

        /**
         * @return The OPER_TYPE of this instance
         */
        OPER_TYPE getOpType() const;

        /**
         * @brief Converts an OPER_TYPE to its string representation
         * @param type The OPER_TYPE to convert
         * @return The string representation of the given OPER_TYPE
         */
        static std::string OperTypeToString(OPER_TYPE type);

        /**
         * @brief Converts a string into an OPER_TYPE
         * @param str The string to convert
         * @return The associated OPER_TYPE
         */
        static OPER_TYPE OperTypeFromString(const std::string& str);

    private:

        static std::unordered_map<std::string, OPER_TYPE> OPERATORS;

        const OPER_TYPE _opType;

    };

}

}

#endif
