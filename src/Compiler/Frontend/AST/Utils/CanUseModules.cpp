//
//  CanUseModules.cpp
//  SFSL
//
//  Created by Romain Beguet on 28.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "CanUseModules.h"

namespace sfsl {

namespace ast {

CanUseModules::~CanUseModules() {

}

void CanUseModules::setUsedModules(const std::vector<CanUseModules::ModulePath>& usedModules) {
    _usedModules = usedModules;
}

const std::vector<CanUseModules::ModulePath>&CanUseModules::getUsedModules() const {
    return _usedModules;
}

}

}
