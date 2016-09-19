//
//  UsageTrackable.h
//  SFSL
//
//  Created by Romain Beguet on 28.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__UsageTrackable__
#define __SFSL__UsageTrackable__

#include <iostream>
#include <type_traits>

#include "../../../../Utils/Utils.h"

#include "../../../Common/Positionnable.h"

namespace sfsl {

namespace ast {

enum class UsageProperty : char {
    DECLARED    = 1 << 0,
    INITIALIZED = 1 << 1,
    USABLE      = 1 << 2,
    USED        = 1 << 3,
    MUTABLE     = 1 << 4,
    ASSIGNED    = 1 << 5
};

/**
 * @brief An interface to keep track of
 * the used state of an object
 */
class UsageTrackable {
protected:

    UsageTrackable();

public:
    void setProperty(UsageProperty property);
    void unsetProperty(UsageProperty property);
    bool hasProperty(UsageProperty property) const;

private:

    UsageProperty _flags;
};

}

template<>
struct enable_bitmask_operators<ast::UsageProperty> {
    static constexpr bool enable = true;
};

}

#endif
