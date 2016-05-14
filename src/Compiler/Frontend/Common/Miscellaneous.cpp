//
//  Miscellanous.cpp
//  SFSL
//
//  Created by Romain Beguet on 11.05.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#include "Miscellaneous.h"

namespace sfsl {

namespace common {

std::string varianceTypeToString(VARIANCE_TYPE vt, bool addSpaceIfNotNone) {
    std::string suffix = addSpaceIfNotNone ? " " : "";
    switch (vt) {
    case VAR_T_IN:  return std::string("in") + suffix;
    case VAR_T_OUT: return std::string("out") + suffix;
    default:        return "";
    }
}

}

}
