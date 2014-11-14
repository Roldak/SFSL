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

OPER_TYPE Operator::getOpType() const {
    return _opType;
}

std::string Operator::OperTypeToString(OPER_TYPE type) {
    switch (type) {

        // BINARY OPERATORS

    case OPER_PLUS:     return "+";
    case OPER_MINUS:    return "-";
    case OPER_TIMES:    return "*";
    case OPER_DIV:      return "/";
    case OPER_POW:      return "^";
    case OPER_AND:      return "&&";
    case OPER_OR:       return "||";
    case OPER_EQ:       return "=";

        // COMPARISON BINARY OPERATORS

    case OPER_EQ_EQ:    return "==";
    case OPER_LT:       return "<";
    case OPER_GT:       return ">";
    case OPER_LE:       return "<=";
    case OPER_GE:       return ">=";

        // BRACKETS

    case OPER_L_PAREN:  return "(";
    case OPER_R_PAREN:  return ")";
    case OPER_L_BRACKET:return "[";
    case OPER_R_BRACKET:return "]";
    case OPER_L_BRACE:  return "{";
    case OPER_R_BRACE:  return "}";

        // SYMBOLS

    case OPER_DOT:      return ".";
    case OPER_COLON:    return ":";
    case OPER_COMMA:    return ",";
    case OPER_SEMICOLON:return ";";

        // OTHERS

    case OPER_THIN_ARROW:return "->";
    case OPER_FAT_ARROW:return "=>";
    case OPER_DOT_DOT:  return "..";

    default:            return "";
    }
}

std::unordered_map<std::string, OPER_TYPE> createOperatorsMap() {
    std::unordered_map<std::string, OPER_TYPE> map;
    map["+"] = OPER_PLUS;
    map["-"] = OPER_MINUS;
    map["*"] = OPER_TIMES;
    map["/"] = OPER_DIV;
    map["^"] = OPER_POW;
    map["&&"] = OPER_AND;
    map["and"] = OPER_AND;
    map["||"] = OPER_OR;
    map["or"] = OPER_OR;
    map["="] = OPER_EQ;
    map["=="] = OPER_EQ_EQ;
    map["<"] = OPER_LT;
    map[">"] = OPER_GT;
    map["<="] = OPER_LE;
    map[">="] = OPER_GE;
    map["("] = OPER_L_PAREN;
    map[")"] = OPER_R_PAREN;
    map["["] = OPER_L_BRACKET;
    map["]"] = OPER_R_BRACKET;
    map["{"] = OPER_L_BRACE;
    map["}"] = OPER_R_BRACE;
    map["."] = OPER_DOT;
    map[":"] = OPER_COLON;
    map[","] = OPER_COMMA;
    map[";"] = OPER_SEMICOLON;
    map["->"] = OPER_THIN_ARROW;
    map["=>"] = OPER_FAT_ARROW;
    map[".."] = OPER_DOT_DOT;
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
