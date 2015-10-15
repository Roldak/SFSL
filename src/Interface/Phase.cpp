//
//  Phase.cpp
//  SFSL
//
//  Created by Romain Beguet on 04.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "api/Phase.h"

namespace sfsl {

const std::string Phase::NoPhase = "";
const std::vector<std::string> Phase::None = {};

Phase::Phase(const std::string& name, const std::string& descr) : _name(name), _descr(descr) {

}

Phase::~Phase() {

}

std::string Phase::runsRightAfter() const {
    return NoPhase;
}

std::string Phase::runsRightBefore() const {
    return NoPhase;
}

std::vector<std::string> Phase::runsAfter() const {
    return None;
}

std::vector<std::string> Phase::runsBefore() const {
    return None;
}

const std::string& Phase::getName() const {
    return _name;
}

const std::string& Phase::getDescr() const {
    return _descr;
}

}
