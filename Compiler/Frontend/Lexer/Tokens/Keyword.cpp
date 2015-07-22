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
    case KW_DEF:        return "def";
    case KW_TPE:       return "type";
    case KW_CLASS:      return "class";
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
    map["def"] = KW_DEF;
    map["type"] = KW_TPE;
    map["class"] = KW_CLASS;
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



}

}
