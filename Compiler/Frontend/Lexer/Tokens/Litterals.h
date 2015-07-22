//
//  Litterals.h
//  SFSL
//
//  Created by Romain Beguet on 13.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Litterals__
#define __SFSL__Litterals__

#include "Token.h"
#include "../../../../Utils/Utils.h"

namespace sfsl {

namespace tok {

    /**
     * @brief Represents an Integer litteral (e.g. 42)
     */
    class IntLitteral : public Token {
    public:

        /**
         * @brief Creates an Integer litteral Token
         * @param value the value of the litteral
         */
        IntLitteral(sfsl_int_t value);
        virtual ~IntLitteral();

        virtual TOK_TYPE getTokenType() const override;
        virtual std::string toString() const override;

        /**
         * @return this litteral's int value
         */
        sfsl_int_t getValue() const;

    private:

        const sfsl_int_t _value;

    };

    /**
     * @brief Represents a Real litteral (e.g. 3.14)
     */
    class RealLitteral : public Token {
    public:

        /**
         * @brief Creates a Real litteral Token
         * @param value the value of the litteral
         */
        RealLitteral(sfsl_real_t value);
        virtual ~RealLitteral();

        virtual TOK_TYPE getTokenType() const override;
        virtual std::string toString() const override;

        /**
         * @return this litteral's real value
         */
        sfsl_real_t getValue() const;

    private:

        const sfsl_real_t _value;

    };

    /**
     * @brief Represents a String litteral (e.g. "hello world")
     */
    class StringLitteral : public Token {
    public:

        /**
         * @brief Creates a String litteral
         * @param value the value of the litteral
         */
        StringLitteral(const std::string& value);
        virtual ~StringLitteral();

        virtual TOK_TYPE getTokenType() const override;
        virtual std::string toString() const override;

        /**
         * @return this litteral's string value
         */
        std::string getValue() const;

    private:

        const std::string _value;

    };

}

}

#endif
