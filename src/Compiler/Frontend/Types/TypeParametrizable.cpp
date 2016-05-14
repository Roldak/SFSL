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

TypeParametrizable::TypeParametrizable() {

}

TypeParametrizable::~TypeParametrizable() {

}

void TypeParametrizable::setDependencies(const std::vector<Parameter>& types) {
    _types = types;
}

const std::vector<TypeParametrizable::Parameter>& TypeParametrizable::getDependencies() const {
    return _types;
}

}

}
