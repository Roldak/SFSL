//
//  Keyword.cpp
//  SFSL
//
//  Created by Romain Beguet on 13.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Keyword.h"

namespace sfsl {

namespace tok {

Keyword::Keyword(KW_TYPE kwType)  : _kwType(kwType) {

}

Keyword::~Keyword() {

}

TOK_TYPE Keyword::getTokenType() {
    return TOK_KW;
}

std::string Keyword::toString() {
    switch (_kwType) {

    default:
        return "";
    }
}

KW_TYPE Keyword::getKwType() {
    return _kwType;
}



}

}
