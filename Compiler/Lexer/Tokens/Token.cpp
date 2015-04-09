//
//  Token.cpp
//  SFSL
//
//  Created by Romain Beguet on 12.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Token.h"
#include "../../../Utils/Utils.h"

namespace sfsl {

namespace tok {

std::string Token::toStringDetailed() const {
    return "{.pos = " + *getSourceName() + "@" + utils::T_toString(getStartPosition())
                      + ":" + utils::T_toString(getEndPosition()) + ", "
            ".type = '" + TokenTypeToString(getTokenType()) +"', "
            ".value = '" + toString() + "'}";
}

std::string Token::TokenTypeToString(TOK_TYPE type) {
    switch (type) {
    case TOK_OPER:      return "OPERATOR";
    case TOK_KW:        return "KEYWORD";
    case TOK_ID:        return "IDENTIFIER";
    case TOK_INT_LIT:   return "INT LITTERAL";
    case TOK_REAL_LIT:  return "REAL LITTERAL";
    case TOK_STR_LIT:   return "STRING LITTERAL";
    case TOK_EOF:       return "EOF";
    case TOK_BAD:       return "BAD";

    default: return "";
    }
}

}

}
