//
//  Tokens.h
//  SFSL
//
//  Created by Romain Beguet on 31.03.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Tokens__
#define __SFSL__Tokens__

#include <unordered_map>

#include "../../Common/MemoryManageable.h"
#include "../../Common/Positionnable.h"
#include "../../../Utils/Utils.h"

namespace sfsl {

namespace tok {

/**
 * @brief Enumerates every possible token type
 */
enum TOK_TYPE { TOK_OPER, TOK_ID, TOK_KW, TOK_BOOL_LIT, TOK_INT_LIT, TOK_REAL_LIT, TOK_STR_LIT, TOK_EOF, TOK_BAD };

/**
 * @brief Represents an abstract Token
 */
class Token : public common::MemoryManageable, public common::Positionnable {
public:

    virtual ~Token();

    /**
     * @return the type of the token
     */
    virtual TOK_TYPE getTokenType() const = 0;

    /**
     * @return a string representation of the token
     */
    virtual std::string toString() const = 0;

    /**
     * @return a string representation of the token with details
     */
    std::string toStringDetailed() const;

    /**
     * @param type the type to represent as a string
     * @return the string representation of the type
     */
    static std::string TokenTypeToString(TOK_TYPE type);

};

/**
 * @brief Represents an identifier
 */
class Identifier : public Token {
public:

    /**
     * @brief Creates an Itentifier Token
     * @param id the name of the identifier
     */
    Identifier(const std::string& id);
    virtual ~Identifier();

    virtual TOK_TYPE getTokenType() const override;
    virtual std::string toString() const override;

private:

    const std::string _id;
};


/**
 * @brief Enumerates every possible Keyword
 */
enum KW_TYPE {
    KW_MODULE, KW_USING, KW_DEF, KW_REDEF, KW_EXTERN, KW_ABSTRACT, KW_TPE, KW_CLASS,
    KW_IN, KW_OUT, KW_NEW, KW_THIS,
    KW_IF, KW_ELSE, KW_WHILE, KW_FOR, KW_UNKNOWN };

/**
 * @brief Represents a Keyword
 */
class Keyword : public Token {
public:

    /**
     * @brief Creates a Keyword Token
     * @param kwType the type of the Keyword
     */
    Keyword(KW_TYPE kwType);
    virtual ~Keyword();

    virtual TOK_TYPE getTokenType() const override;
    virtual std::string toString() const override;

    /**
     * @return the Keyword type of this particular instance
     */
    KW_TYPE getKwType();

    /**
     * @param type the Keyword type to convert
     * @return the string representation of the keyword type
     */
    static std::string KeywordTypeToString(KW_TYPE type);

    /**
     * @param str the string representation of the keyword type
     * @return the associated Keyword type
     */
    static KW_TYPE KeywordTypeFromString(const std::string& str);

private:

    static std::unordered_map<std::string, KW_TYPE> KEYWORDS;

    const KW_TYPE _kwType;

};

/**
 * @brief Represents a Boolean Literal (true or false)
 */
class BoolLiteral : public Token {
public:

    /**
     * @brief Creates a Boolean Literal Token
     * @param value the value of the Literal
     */
    BoolLiteral(sfsl_bool_t value);
    virtual ~BoolLiteral();

    virtual TOK_TYPE getTokenType() const override;
    virtual std::string toString() const override;

    sfsl_bool_t getValue() const;

private:

    const sfsl_bool_t _value;
};

/**
 * @brief Represents an Integer Literal (e.g. 42)
 */
class IntLiteral : public Token {
public:

    /**
     * @brief Creates an Integer Literal Token
     * @param value the value of the Literal
     */
    IntLiteral(sfsl_int_t value);
    virtual ~IntLiteral();

    virtual TOK_TYPE getTokenType() const override;
    virtual std::string toString() const override;

    /**
     * @return this Literal's int value
     */
    sfsl_int_t getValue() const;

private:

    const sfsl_int_t _value;

};

/**
 * @brief Represents a Real Literal (e.g. 3.14)
 */
class RealLiteral : public Token {
public:

    /**
     * @brief Creates a Real Literal Token
     * @param value the value of the Literal
     */
    RealLiteral(sfsl_real_t value);
    virtual ~RealLiteral();

    virtual TOK_TYPE getTokenType() const override;
    virtual std::string toString() const override;

    /**
     * @return this Literal's real value
     */
    sfsl_real_t getValue() const;

private:

    const sfsl_real_t _value;

};

/**
 * @brief Represents a String Literal (e.g. "hello world")
 */
class StringLiteral : public Token {
public:

    /**
     * @brief Creates a String Literal
     * @param value the value of the Literal
     */
    StringLiteral(const std::string& value);
    virtual ~StringLiteral();

    virtual TOK_TYPE getTokenType() const override;
    virtual std::string toString() const override;

    /**
     * @return this Literal's string value
     */
    std::string getValue() const;

private:

    const std::string _value;

};

/**
 * @brief Enumerates every possible Operator type
 */
enum OPER_TYPE {
    OPER_PLUS = 0, OPER_MINUS, OPER_TIMES, OPER_DIV, OPER_MOD, OPER_POW, OPER_AND, OPER_OR,
        OPER_BIT_AND, OPER_BIT_OR, OPER_L_SHIFT, OPER_R_SHIFT, // BINARY OPERATORS
    OPER_EQ, // ASSIGNMENT OPERATORS
    OPER_BANG, OPER_TILDE, // UNARY OPERATORS
    OPER_EQ_EQ, OPER_NOT_EQ, OPER_LT, OPER_GT, OPER_LE, OPER_GE, // COMPARISON OPERATORS
    OPER_L_PAREN, OPER_R_PAREN, OPER_L_BRACKET, OPER_R_BRACKET, OPER_L_BRACE, OPER_R_BRACE, // BRACKETS
    OPER_DOT, OPER_COLON, OPER_COMMA, OPER_SEMICOLON, // SYMBOLS
    OPER_THIN_ARROW, OPER_FAT_ARROW, OPER_DOT_DOT, OPER_AT, OPER_SHARP, // OTHERS
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
     * @brief All the unary operators have the same precedence,
     * so there is no need to store the precedence of unary operators
     * in the precedence table
     *
     * @return The precedence of any unary operator
     */
    static int getUnaryOperatorPrecedence();

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

    virtual TOK_TYPE getTokenType() const override;
    virtual std::string toString() const override;

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

    virtual TOK_TYPE getTokenType() const override;
    virtual std::string toString() const override;

private:

    const std::string _str;
};

}

}

#endif
