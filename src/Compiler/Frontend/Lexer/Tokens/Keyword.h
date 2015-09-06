//
//  Keyword.h
//  SFSL
//
//  Created by Romain Beguet on 13.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Keyword__
#define __SFSL__Keyword__

#include <unordered_map>
#include "Token.h"

namespace sfsl {

namespace tok {

/**
 * @brief Enumerates every possible Keyword
 */
enum KW_TYPE { KW_MODULE, KW_DEF, KW_REDEF, KW_TPE, KW_CLASS, KW_THIS, KW_IF, KW_ELSE, KW_WHILE, KW_FOR, KW_UNKNOWN };

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

}

}

#endif
