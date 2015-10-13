//
//  PhaseGraph.cpp
//  SFSL
//
//  Created by Romain Beguet on 12.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include <map>
#include "PhaseGraph.h"

namespace sfsl {

typedef std::shared_ptr<Phase> PhasePtr;

struct PhaseNode final {
    PhaseNode(std::vector<PhasePtr>& phases) : phases(phases) { }
    ~PhaseNode() { }

    std::vector<PhasePtr> phases;
    std::vector<PhaseNode*> edges;
};

struct PhaseEdge final {
    PhaseEdge(PhasePtr from, PhasePtr to, bool isHard) : from(from), to(to), isHard(isHard) { }
    ~PhaseEdge() { }

    PhasePtr from;
    PhasePtr to;
    bool isHard;
};

class PhaseGraph final {
public:
    PhaseGraph(std::set<std::shared_ptr<Phase>>& phases);
    ~PhaseGraph() { }

    std::vector<std::shared_ptr<Phase>> sort();

private:

    PhasePtr phaseForName(const std::string& phaseName);

    std::set<PhaseNode> prepare();

    std::set<PhasePtr> _phases;
    std::map<std::string, PhasePtr> _nameMap;
};

PhaseGraph::PhaseGraph(std::set<std::shared_ptr<Phase>>& phases) : _phases(phases) {
    for (PhasePtr phase : _phases) {
        _nameMap[phase->getName()] = phase;
    }
}

std::vector<std::shared_ptr<Phase>> PhaseGraph::sort() {

}

PhasePtr PhaseGraph::phaseForName(const std::string& phaseName) {
    if (!phaseName.empty()) {
        auto it = _nameMap.find(phaseName);
        if (it != _nameMap.end()) {
            return it->second;
        }
    }
    return PhasePtr(nullptr);
}

std::set<PhaseEdge> PhaseGraph::prepare() {
    std::set<PhaseEdge> edges;

    for (PhasePtr phase : _phases) {
        if (PhasePtr rightAfter = phaseForName(phase->runsRightAfter())) {
            edges.insert(PhaseEdge(phase, rightAfter, true));
        }
        if (PhasePtr rightBefore = phaseForName(phase->runsRightBefore())) {
            edges.insert(PhaseEdge(phase, rightBefore, true);
        }

        for (const std::string& phase : phase->runsAfter()) {
            if (PhasePtr after = phaseForName(phase)) {
                edges.insert(PhaseEdge(phase, after, false));
            }
        }

        for (const std::string& phase : phase->runsBefore()) {
            if (PhasePtr before = phaseForName(phase)) {
                edges.insert(PhaseEdge(phase, before, false);
            }
        }
    }

    return edges;
}

std::vector<PhasePtr> sortPhases(std::set<std::shared_ptr<Phase>>& phases) {

}



}
