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

UsageTrackable::UsageTrackable() {
    unsetProperty(UsageProperty::DECLARED);
    unsetProperty(UsageProperty::INITIALIZED);
      setProperty(UsageProperty::USABLE);
    unsetProperty(UsageProperty::USED);
    unsetProperty(UsageProperty::MUTABLE);
}

void UsageTrackable::setProperty(UsageProperty property) {
    _flags |= property;
}

void UsageTrackable::unsetProperty(UsageProperty property) {
    _flags &= ~property;
}

bool UsageTrackable::hasProperty(UsageProperty property) const {
    return static_cast<bool>(_flags & property);
}

}

}
