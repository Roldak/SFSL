//
//  Miscellanous.h
//  SFSL
//
//  Created by Romain Beguet on 11.05.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Miscellanous__
#define __SFSL__Miscellanous__

#include <iostream>

namespace sfsl {

namespace common {

enum VARIANCE_TYPE {
    VAR_T_NONE,
    VAR_T_IN,
    VAR_T_OUT
};

std::string varianceTypeToString(VARIANCE_TYPE vt, bool addSpaceIfNotNone = false);

}

}

#endif
