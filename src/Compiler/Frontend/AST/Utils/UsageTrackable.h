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

    /**
     * @brief Sets the used state to the given value
     */
    void setUsed(bool val);

    /**
     * @return True if the object has been used, otherwise false
     */
    bool isUsed() const;

private:

    bool _used;
};

}

}

#endif
