//
//  Phase.cpp
//  SFSL
//
//  Created by Romain Beguet on 04.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "api/Phase.h"

namespace sfsl {

class PhaseStopAfter : public Phase {
public:
    PhaseStopAfter(const std::string& phase)
        : Phase("_StopAfter_", "Phase that makes the compilation stop after phase " + phase), _phase(phase) { }

    virtual ~PhaseStopAfter() { }

    virtual std::string runsRightAfter() const { return _phase; }

    virtual bool run(PhaseContext &pctx) {
        return false;
    }

private:

    std::string _phase;
};

Phase::Phase(const std::string& name, const std::string& descr) : _name(name), _descr(descr) {

}

Phase::~Phase() {

}

std::string Phase::runsRightAfter() const {
    return "";
}

std::string Phase::runsRightBefore() const {
    return "";
}

std::vector<std::string> Phase::runsAfter() const {
    return {};
}

std::vector<std::string> Phase::runsBefore() const {
    return {};
}

const std::string& Phase::getName() const {
    return _name;
}

const std::string& Phase::getDescr() const {
    return _descr;
}

std::shared_ptr<Phase> Phase::StopRightAfter(const std::string& phase) {
    return std::shared_ptr<Phase>(new PhaseStopAfter(phase));
}

}
