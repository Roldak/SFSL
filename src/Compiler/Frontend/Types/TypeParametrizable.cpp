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

void TypeParametrizable::setDependencies(const std::vector<sym::TypeSymbol*>& types) {
    _types = types;
}

const std::vector<sym::TypeSymbol*>& TypeParametrizable::getDependencies() const {
    return _types;
}

}

}
