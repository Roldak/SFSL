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

    enum KW_TYPE { KW_MODULE };

    /**
     * @brief Represents a Keyword
     */
    class Keyword : public Token {
    public:

        Keyword(KW_TYPE kwType);
        virtual ~Keyword();

        virtual TOK_TYPE getTokenType() const;
        virtual std::string toString() const;

        /**
         * @return the keyword type (#KW_TYPE) of this particular instance
         */
        KW_TYPE getKwType();

        /**
         * @param type the #KW_TYPE to convert
         * @return the string representation of the keyword type
         */
        static std::string KeywordTypeToString(KW_TYPE type);

        /**
         * @param str the string representation of the keyword type
         * @return the associated #KW_TYPE
         */
        static KW_TYPE KeywordTypeFromString(const std::string& str);

    private:

        static std::unordered_map<std::string, KW_TYPE> KEYWORDS;

        const KW_TYPE _kwType;

    };

}

}

#endif
