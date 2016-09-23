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

// HAS CACHEABLE CREATED TYPE

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

// CAN HOLD DEFAULT TYPE

CanHoldDefaultType::CanHoldDefaultType() : _defaultType(nullptr) {

}

CanHoldDefaultType::~CanHoldDefaultType() {

}

bool CanHoldDefaultType::holdsDefaultType() const {
    return _defaultType != nullptr;
}

TypeExpression* CanHoldDefaultType::getDefaultType() const {
    return _defaultType;
}

void CanHoldDefaultType::setDefaultType(TypeExpression* texpr) {
    _defaultType = texpr;
}

}

}
