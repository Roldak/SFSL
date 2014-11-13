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

TOK_TYPE Operator::getTokenType() const {
    return TOK_OPER;
}

std::string Operator::toString() const {
    return OperTypeToString(_opType);
}

std::string Operator::OperTypeToString(OPER_TYPE type) {
    switch (type) {
    case OPER_PLUS:     return "+";
    default:            return "";
    }
}

std::unordered_map<std::string, OPER_TYPE> createOperatorsMap() {
    std::unordered_map<std::string, OPER_TYPE> map;
    map["+"] = OPER_PLUS;
    return map;
}


std::unordered_map<std::string, OPER_TYPE> Operator::OPERATORS = createOperatorsMap();


OPER_TYPE Operator::OperTypeFromString(const std::string &str) {
    const auto& it = OPERATORS.find(str);

    if (it != OPERATORS.end()) {
        return (*it).second;
    } else {
        return OPER_UNKNOWN;
    }
}

}


}
