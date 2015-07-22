//
//  Bytecode.cpp
//  SFSL
//
//  Created by Romain Beguet on 22.07.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "Bytecode.h"
#include "../../Utils/Utils.h"

namespace sfsl {

namespace bc {

BCInstruction::~BCInstruction() {

}

std::string BCInstruction::toStringDetailed() const {
    return "<" + getSourceName().getName() + "@" + utils::T_toString(getStartPosition())
                      + ":" + utils::T_toString(getEndPosition()) + ">\t" + toString();
}

}

}
