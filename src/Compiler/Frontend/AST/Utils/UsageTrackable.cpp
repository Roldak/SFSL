//
//  UsageTrackable.cpp
//  SFSL
//
//  Created by Romain Beguet on 28.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "UsageTrackable.h"

namespace sfsl {

namespace ast {

UsageTrackable::UsageTrackable() : _flags(0) {
    setProperty(USABLE);
}

void UsageTrackable::setProperty(PROPERTY_TYPE property) {
    _flags |= property;
}

void UsageTrackable::unsetProperty(UsageTrackable::PROPERTY_TYPE property) {
    _flags &= ~property;
}

bool UsageTrackable::getProperty(PROPERTY_TYPE property) {
    return _flags & property;
}

}

}
