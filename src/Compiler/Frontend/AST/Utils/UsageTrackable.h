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
#include <vector>

#include "../../../Common/Positionnable.h"

namespace sfsl {

namespace ast {

/**
 * @brief An interface to keep track of
 * the used state of an object
 */
class UsageTrackable {
protected:

    UsageTrackable();

public:

    enum PROPERTY_TYPE {
        DECLARED    = 1 << 0,
        INITIALIZED = 1 << 1,
        USABLE      = 1 << 2,
        USED        = 1 << 3,
        MUTABLE     = 1 << 4
    };

    void setProperty(PROPERTY_TYPE property);
    void unsetProperty(PROPERTY_TYPE property);
    bool getProperty(PROPERTY_TYPE property) const;

private:

    char _flags;
};

}

}

#endif
