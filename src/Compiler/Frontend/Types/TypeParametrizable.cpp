//
//  TypeParametrizable.cpp
//  SFSL
//
//  Created by Romain Beguet on 06.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "TypeParametrizable.h"

namespace sfsl {

namespace type {

TypeParametrizable::Parameter::Parameter() : varianceType(common::VAR_T_NONE), symbol(nullptr) {

}

TypeParametrizable::Parameter::Parameter(common::VARIANCE_TYPE varianceType, sym::TypeSymbol* symbol)
    : varianceType(varianceType), symbol(symbol) {

}

TypeParametrizable::~TypeParametrizable() {

}

void TypeParametrizable::setParameters(const std::vector<Parameter>& types) {
    _types = types;
}

const std::vector<TypeParametrizable::Parameter>& TypeParametrizable::getParameters() const {
    return _types;
}

}

}
