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

}

PhaseContext& PhaseContext::output(const std::string& name, IOPhaseObject* object) {
    setIOPhaseObject(name, object);
    return *this;
}

IOPhaseObject* PhaseContext::findIOPhaseObject(const std::string& name) const {
    auto it = _phaseObjects.find(name);
    if (it != _phaseObjects.end()) {
        return it->second;
    }
    return nullptr;
}

void PhaseContext::setIOPhaseObject(const std::string& name, IOPhaseObject* obj) {
    _phaseObjects[name] = obj;
}

}
