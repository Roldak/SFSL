//
//  Scoped.cpp
//  SFSL
//
//  Created by Romain Beguet on 13.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Scoped.h"

namespace sfsl {

namespace sym {

Scoped::~Scoped() {

}

void Scoped::setScope(Scope* scope) {
    _scope = scope;
}

Scope* Scoped::getScope() const {
    return _scope;
}

}

}
