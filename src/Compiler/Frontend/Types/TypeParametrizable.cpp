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

TypeParametrizable::TypeParametrizable() {

}

TypeParametrizable::~TypeParametrizable() {

}

void TypeParametrizable::setDependsOn(sym::TypeSymbol* type) {
    _types.push_back(type);
}

const std::vector<sym::TypeSymbol*>&TypeParametrizable::getDependencies() const {
    return _types;
}

}

}
