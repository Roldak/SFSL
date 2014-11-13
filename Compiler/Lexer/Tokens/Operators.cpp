//
//  Operators.cpp
//  SFSL
//
//  Created by Romain Beguet on 13.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Operators.h"

namespace sfsl {

namespace tok {

Operator::Operator(OPER_TYPE opType) : _opType(opType) {

}

Operator::~Operator() {

}

TOK_TYPE Operator::getTokenType() {
    return TOK_OPER;
}

std::string Operator::toString() {
    switch (_opType) {

    default:
        return "";
    }
}



}

}
