//
//  CodeGenOutput.cpp
//  SFSL
//
//  Created by Romain Beguet on 21.07.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "CodeGenOutput.h"

namespace sfsl {

namespace out {

// CURSOR

Cursor::Cursor() {

}

Cursor::~Cursor() {

}

bool Cursor::equivalent(Cursor*) const {
    return false;
}

}

}
