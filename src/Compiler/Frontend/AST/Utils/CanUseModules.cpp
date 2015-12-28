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

CanUseModules::ModulePath::~ModulePath() {

}

void CanUseModules::ModulePath::push_back(const std::string& pathUnit) {
    _pathUnits.push_back(pathUnit);
}

const std::string& CanUseModules::ModulePath::operator[](size_t index) const {
    return _pathUnits[index];
}

size_t CanUseModules::ModulePath::size() const {
    return _pathUnits.size();
}

std::string CanUseModules::ModulePath::toString() const {
    return toString(size());
}

std::string CanUseModules::ModulePath::toString(size_t toElemIndex) const {
    if (toElemIndex == 0) {
        return "";
    }

    std::string toRet = "";

    for (size_t i = 0; i < toElemIndex - 1; ++i) {
        toRet += _pathUnits[i] + ".";
    }

    return toRet + _pathUnits[toElemIndex - 1];
}

CanUseModules::CanUseModules() {

}

CanUseModules::~CanUseModules() {

}

void CanUseModules::setUsedModules(const std::vector<CanUseModules::ModulePath>& usedModules) {
    _usedModules = usedModules;
}

const std::vector<CanUseModules::ModulePath>& CanUseModules::getUsedModules() const {
    return _usedModules;
}

}

}
