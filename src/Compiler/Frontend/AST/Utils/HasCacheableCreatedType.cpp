//
//  HasCacheableCreatedType.cpp
//  SFSL
//
//  Created by Romain Beguet on 23.08.16.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "HasCacheableCreatedType.h"

namespace sfsl {

namespace ast {

HasCacheableCreatedType::HasCacheableCreatedType() : _cached(nullptr) {

}

HasCacheableCreatedType::~HasCacheableCreatedType() {

}

bool HasCacheableCreatedType::hasCachedType() const {
    return _cached;
}

type::Type* HasCacheableCreatedType::getCachedType() const {
    return _cached;
}

void HasCacheableCreatedType::setCachedType(type::Type* tp) {
    _cached = tp;
}

}

}
