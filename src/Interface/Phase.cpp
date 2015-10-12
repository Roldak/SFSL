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

const std::string Phase::runsRightAfter() const {
    return NoPhase;
}

const std::string Phase::runsRightBefore() const {
    return NoPhase;
}

const std::vector<std::string> Phase::runsAfter() const {
    return None;
}

const std::vector<std::string> Phase::runsBefore() const {
    return None;
}

}
