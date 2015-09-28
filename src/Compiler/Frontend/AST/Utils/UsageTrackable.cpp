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

UsageTrackable::UsageTrackable() : _used(false) {}

void UsageTrackable::setUsed(bool val) {
    _used = val;
}

bool UsageTrackable::isUsed() const {
    return _used;
}

}

}
