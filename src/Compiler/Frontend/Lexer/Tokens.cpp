//
//  Tokens.cpp
//  SFSL
//
//  Created by Romain Beguet on 31.03.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#include "Tokens.h"
#include "../../../Utils/Utils.h"

namespace sfsl {

namespace tok {

// TOKEN

Token::~Token() {

}

std::string Token::toStringDetailed() const {
    return "{.pos = " + getSourceName().getName() + "@" + utils::T_toString(getStartPosition())
                      + ":" + utils::T_toString(getEndPosition()) + ", "
            ".type = '" + TokenTypeToString(getTokenType()) +"', "
            ".value = '" + toString() + "'}";
}

std::string Token::TokenTypeToString(TOK_TYPE type) {
    switch (type) {
    case TOK_OPER:      return "OPERATOR";
    case TOK_KW:        return "KEYWORD";
    case TOK_ID:        return "IDENTIFIER";
    case TOK_INT_LIT:   return "INT Literal";
    case TOK_REAL_LIT:  return "REAL Literal";
    case TOK_STR_LIT:   return "STRING Literal";
    case TOK_EOF:       return "EOF";
    case TOK_BAD:       return "BAD";

    default: return "";
    }
}

// IDENTIFIER

Identifier::Identifier(const std::string &id)  : _id(id) {

}

Identifier::~Identifier() {

}

TOK_TYPE Identifier::getTokenType() const {
    return TOK_ID;
}

std::string Identifier::toString() const {
    return _id;
}

// KEYWORD

Keyword::Keyword(KW_TYPE kwType)  : _kwType(kwType) {

}

Keyword::~Keyword() {

}

TOK_TYPE Keyword::getTokenType() const {
    return TOK_KW;
}

std::string Keyword::toString() const {
    return KeywordTypeToString(_kwType);
}

KW_TYPE Keyword::getKwType() {
    return _kwType;
}

std::string Keyword::KeywordTypeToString(KW_TYPE type) {
    switch (type) {
    case KW_MODULE:     return "module";
    case KW_USING:      return "using";
    case KW_DEF:        return "def";
    case KW_REDEF:      return "redef";
    case KW_EXTERN:     return "extern";
    case KW_ABSTRACT:   return "abstract";
    case KW_STATIC:     return "static";
    case KW_TPE:        return "type";
    case KW_CLASS:      return "class";
    case KW_IN:         return "in";
    case KW_OUT:        return "out";
    case KW_NEW:        return "new";
    case KW_THIS:       return "this";
    case KW_IF:         return "if";
    case KW_ELSE:       return "else";
    case KW_WHILE:      return "while";
    case KW_FOR:        return "for";
    default:            return "";
    }
}

std::unordered_map<std::string, KW_TYPE> createKeywordsMap() {
    std::unordered_map<std::string, KW_TYPE> map;
    map["module"] = KW_MODULE;
    map["using"] = KW_USING;
    map["def"] = KW_DEF;
    map["redef"] = KW_REDEF;
    map["extern"] = KW_EXTERN;
    map["abstract"] = KW_ABSTRACT;
    map["static"] = KW_STATIC;
    map["type"] = KW_TPE;
    map["class"] = KW_CLASS;
    map["in"] = KW_IN;
    map["out"] = KW_OUT;
    map["new"] = KW_NEW;
    map["this"] = KW_THIS;
    map["if"] = KW_IF;
    map["else"] = KW_ELSE;
    map["while"] = KW_WHILE;
    map["for"] = KW_FOR;
    return map;
}

std::unordered_map<std::string, KW_TYPE> Keyword::KEYWORDS = createKeywordsMap();

KW_TYPE Keyword::KeywordTypeFromString(const std::string& str) {
    const auto& it = KEYWORDS.find(str);

    if (it != KEYWORDS.end()) {
        return (*it).second;
    } else {
        return KW_UNKNOWN;
    }
}

// BOOLS

BoolLiteral::BoolLiteral(sfsl_bool_t value) : _value(value) {

}

BoolLiteral::~BoolLiteral() {

}

TOK_TYPE BoolLiteral::getTokenType() const {
    return TOK_BOOL_LIT;
}

std::string BoolLiteral::toString() const {
    return _value ? "true" : "false";
}

sfsl_bool_t BoolLiteral::getValue() const {
    return _value;
}

// INTS

IntLiteral::IntLiteral(sfsl_int_t value)  : _value(value) {

}

IntLiteral::~IntLiteral() {

}

TOK_TYPE IntLiteral::getTokenType() const {
    return TOK_INT_LIT;
}

std::string IntLiteral::toString() const {
    return utils::T_toString(_value);
}

sfsl_int_t IntLiteral::getValue() const {
    return _value;
}

// REALS

RealLiteral::RealLiteral(sfsl_real_t value) : _value(value) {

}

RealLiteral::~RealLiteral() {

}

TOK_TYPE RealLiteral::getTokenType() const {
    return TOK_REAL_LIT;
}

std::string RealLiteral::toString() const {
    return utils::T_toString(_value);
}

sfsl_real_t RealLiteral::getValue() const {
    return _value;
}

// STRS

StringLiteral::StringLiteral(const std::string &value) : _value(value) {

}

StringLiteral::~StringLiteral() {

}

TOK_TYPE StringLiteral::getTokenType() const {
    return TOK_STR_LIT;
}

std::string StringLiteral::toString() const {
    return "\"" + _value + "\"";
}

std::string StringLiteral::getValue() const {
    return _value;
}

// OPERATORS

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

int Operator::getPrecedence() const {
    return PRECEDENCE[_opType];
}

bool Operator::isRightAssociative() const {
    // only assignment operators and power operator are right associative
    return (_opType >= OPER_EQ && _opType <= OPER_R_SHIFT_EQ) || _opType == OPER_POW;
}

int Operator::getUnaryOperatorPrecedence() {
    return 60;
}

std::string Operator::OperTypeToString(OPER_TYPE type) {
    switch (type) {

        // BINARY OPERATORS

    case OPER_PLUS:         return "+";
    case OPER_MINUS:        return "-";
    case OPER_TIMES:        return "*";
    case OPER_DIV:          return "/";
    case OPER_MOD:          return "%";
    case OPER_POW:          return "^";
    case OPER_AND:          return "&&";
    case OPER_OR:           return "||";
    case OPER_BIT_AND:      return "&";
    case OPER_BIT_OR:       return "|";
    case OPER_L_SHIFT:      return "<<";
    case OPER_R_SHIFT:      return ">>";

        // ASSIGNMENT OPERATORS

    case OPER_EQ:           return "=";
    case OPER_PLUS_EQ:      return "+=";
    case OPER_MINUS_EQ:     return "-=";
    case OPER_TIMES_EQ:     return "*=";
    case OPER_DIV_EQ:       return "/=";
    case OPER_MOD_EQ:       return "%=";
    case OPER_POW_EQ:       return "^=";
    case OPER_B_AND_EQ:     return "&=";
    case OPER_B_OR_EQ:      return "|=";
    case OPER_L_SHIFT_EQ:   return "<<=";
    case OPER_R_SHIFT_EQ:   return ">>=";

        // UNARY OPERATORS

    case OPER_BANG:         return "!";
    case OPER_TILDE:        return "~";

        // COMPARISON BINARY OPERATORS

    case OPER_EQ_EQ:        return "==";
    case OPER_NOT_EQ:       return "!=";
    case OPER_LT:           return "<";
    case OPER_GT:           return ">";
    case OPER_LE:           return "<=";
    case OPER_GE:           return ">=";

        // BRACKETS

    case OPER_L_PAREN:      return "(";
    case OPER_R_PAREN:      return ")";
    case OPER_L_BRACKET:    return "[";
    case OPER_R_BRACKET:    return "]";
    case OPER_L_BRACE:      return "{";
    case OPER_R_BRACE:      return "}";

        // SYMBOLS

    case OPER_DOT:          return ".";
    case OPER_COLON:        return ":";
    case OPER_COMMA:        return ",";
    case OPER_SEMICOLON:    return ";";

        // OTHERS

    case OPER_THIN_ARROW:   return "->";
    case OPER_FAT_ARROW:    return "=>";
    case OPER_DOT_DOT:      return "..";
    case OPER_AT:           return "@";
    case OPER_SHARP:        return "#";

    default:                return "";
    }
}

std::unordered_map<std::string, OPER_TYPE> createOperatorsMap() {
    std::unordered_map<std::string, OPER_TYPE> map;

    map["+"]    = OPER_PLUS;
    map["-"]    = OPER_MINUS;
    map["*"]    = OPER_TIMES;
    map["/"]    = OPER_DIV;
    map["%"]    = OPER_MOD;
    map["^"]    = OPER_POW;
    map["&&"]   = OPER_AND;
    map["and"]  = OPER_AND;
    map["||"]   = OPER_OR;
    map["or"]   = OPER_OR;
    map["&"]    = OPER_BIT_AND;
    map["|"]    = OPER_BIT_OR;
    map["<<"]   = OPER_L_SHIFT;
    map[">>"]   = OPER_R_SHIFT;

    map["="]    = OPER_EQ;
    map["+="]   = OPER_PLUS_EQ;
    map["-="]   = OPER_MINUS_EQ;
    map["*="]   = OPER_TIMES_EQ;
    map["/="]   = OPER_DIV_EQ;
    map["%="]   = OPER_MOD_EQ;
    map["^="]   = OPER_POW_EQ;
    map["&="]   = OPER_B_AND_EQ;
    map["|="]   = OPER_B_OR_EQ;
    map["<<="]  = OPER_L_SHIFT_EQ;
    map[">>="]  = OPER_R_SHIFT_EQ;

    map["!"]    = OPER_BANG;
    map["not"]  = OPER_BANG;
    map["~"]    = OPER_TILDE;

    map["=="]   = OPER_EQ_EQ;
    map["!="]   = OPER_NOT_EQ;
    map["<"]    = OPER_LT;
    map[">"]    = OPER_GT;
    map["<="]   = OPER_LE;
    map[">="]   = OPER_GE;

    map["("]    = OPER_L_PAREN;
    map[")"]    = OPER_R_PAREN;
    map["["]    = OPER_L_BRACKET;
    map["]"]    = OPER_R_BRACKET;
    map["{"]    = OPER_L_BRACE;
    map["}"]    = OPER_R_BRACE;

    map["."]    = OPER_DOT;
    map[":"]    = OPER_COLON;
    map[","]    = OPER_COMMA;
    map[";"]    = OPER_SEMICOLON;

    map["->"]   = OPER_THIN_ARROW;
    map["=>"]   = OPER_FAT_ARROW;
    map[".."]   = OPER_DOT_DOT;
    map["@"]    = OPER_AT;
    map["#"]    = OPER_SHARP;

    return map;
}

std::unordered_map<std::string, OPER_TYPE> Operator::OPERATORS = createOperatorsMap();

std::vector<int> createOperatorsPrecedenceTable() {
    std::vector<int> prec(OPER_UNKNOWN, -1);
    prec[OPER_L_PAREN]      = 100;
    prec[OPER_L_BRACKET]    = 100;
    prec[OPER_THIN_ARROW]   = 100;
    prec[OPER_FAT_ARROW]    = 80;
    prec[OPER_DOT]          = 70;
    prec[OPER_COLON]        = 65;
    prec[OPER_POW]          = 50;
    prec[OPER_TIMES]        = 40;
    prec[OPER_DIV]          = 40;
    prec[OPER_MOD]          = 40;
    prec[OPER_PLUS]         = 20;
    prec[OPER_MINUS]        = 20;
    prec[OPER_L_SHIFT]      = 15;
    prec[OPER_R_SHIFT]      = 15;
    prec[OPER_GT]           = 10;
    prec[OPER_LT]           = 10;
    prec[OPER_GE]           = 10;
    prec[OPER_LE]           = 10;
    prec[OPER_EQ_EQ]        = 9;
    prec[OPER_NOT_EQ]       = 9;
    prec[OPER_BIT_AND]      = 8;
    prec[OPER_BIT_OR]       = 7;
    prec[OPER_AND]          = 6;
    prec[OPER_OR]           = 5;
    prec[OPER_EQ]           = 2;
    prec[OPER_PLUS_EQ]      = 2;
    prec[OPER_MINUS_EQ]     = 2;
    prec[OPER_TIMES_EQ]     = 2;
    prec[OPER_DIV_EQ]       = 2;
    prec[OPER_MOD_EQ]       = 2;
    prec[OPER_POW_EQ]       = 2;
    prec[OPER_B_AND_EQ]     = 2;
    prec[OPER_B_OR_EQ]      = 2;
    prec[OPER_L_SHIFT_EQ]   = 2;
    prec[OPER_R_SHIFT_EQ]   = 2;
    return prec;
}

std::vector<int> Operator::PRECEDENCE = createOperatorsPrecedenceTable();

OPER_TYPE Operator::OperTypeFromString(const std::string &str) {
    const auto& it = OPERATORS.find(str);

    if (it != OPERATORS.end()) {
        return (*it).second;
    } else {
        return OPER_UNKNOWN;
    }
}

OPER_TYPE Operator::OperTypeFromIdentifierString(const std::string &id) {
    if (id == "and") return OPER_AND;
    else if (id == "or") return OPER_OR;
    else if (id == "not") return OPER_BANG;
    else return OPER_UNKNOWN;
}


// EOF

EOFToken::EOFToken() {

}

EOFToken::~EOFToken() {

}

TOK_TYPE EOFToken::getTokenType() const {
    return TOK_EOF;
}

std::string EOFToken::toString() const {
    return "EOF";
}

// BAD

BadToken::BadToken(const std::string& str) : _str(str) {

}

BadToken::~BadToken() {

}

TOK_TYPE BadToken::getTokenType() const {
    return TOK_BAD;
}

std::string BadToken::toString() const {
    return _str;
}

}

}
