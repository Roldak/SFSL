//
//  Operators.h
//  SFSL
//
//  Created by Romain Beguet on 13.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Operators__
#define __SFSL__Operators__

#include <vector>
#include <unordered_map>
#include "Token.h"

namespace sfsl {

namespace tok {

    /**
     * @brief Enumerates every possible Operator type
     */
    enum OPER_TYPE {
        OPER_PLUS = 0, OPER_MINUS, OPER_TIMES, OPER_DIV, OPER_MOD, OPER_POW, OPER_AND, OPER_OR, OPER_EQ, OPER_COL_EQ, // BINARY OPERATORS
        OPER_EQ_EQ, OPER_NOT_EQ, OPER_LT, OPER_GT, OPER_LE, OPER_GE, // COMPARISON OPERATORS
        OPER_L_PAREN, OPER_R_PAREN, OPER_L_BRACKET, OPER_R_BRACKET, OPER_L_BRACE, OPER_R_BRACE, // BRACKETS
        OPER_DOT, OPER_COLON, OPER_COMMA, OPER_SEMICOLON, // SYMBOLS
        OPER_THIN_ARROW, OPER_FAT_ARROW, OPER_DOT_DOT, OPER_SHARP, // OTHERS
        OPER_UNKNOWN };

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

        virtual TOK_TYPE getTokenType() const override;
        virtual std::string toString() const override;

        /**
         * @return The OPER_TYPE of this instance
         */
        OPER_TYPE getOpType() const;

        /**
         * @return The precedence of this operator
         */
        int getPrecedence() const;

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

        /**
         * @brief Converts a string which holds an identifier kind of content
         * into an OPER_TYPE, e.g. "and", "or", "not"
         * @param id The string to convert
         * @return The associated OPER_TYPE
         */
        static OPER_TYPE OperTypeFromIdentifierString(const std::string& id);

    private:

        static std::unordered_map<std::string, OPER_TYPE> OPERATORS;
        static std::vector<int> PRECEDENCE;

        const OPER_TYPE _opType;

    };

}

}

#endif
