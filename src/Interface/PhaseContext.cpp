//
//  PhaseContext.cpp
//  SFSL
//
//  Created by Romain Beguet on 15.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "api/PhaseContext.h"

namespace sfsl {

PhaseContext::PhaseContext() {

}

PhaseContext::~PhaseContext() {
    for (const auto& pair : _phaseObjects) {
        delete pair.second;
    }
}

IOPhaseObject* PhaseContext::findIOPhaseObject(const std::string& name) const {
    auto it = _phaseObjects.find(name);
    if (it != _phaseObjects.end()) {
        return it->second;
    }
    return nullptr;
}

void PhaseContext::setIOPhaseObject(const std::string& name, IOPhaseObject* obj) {
    IOPhaseObject*& old = _phaseObjects[name];
    if (old) {
        delete old;
    }
    old = obj;
}

}
