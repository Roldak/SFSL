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
    PhaseNode() { }
    ~PhaseNode() { }

    std::vector<PhasePtr> phases;
    std::vector<PhaseNode*> edges;
};

struct PhaseEdge final {
    PhaseEdge(PhasePtr from, PhasePtr to, bool isHard) : from(from), to(to), isHard(isHard) { }
    ~PhaseEdge() { }

    bool operator<(const PhaseEdge& other) {
        return from < other.from;
    }

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

    std::vector<PhaseEdge> createEdges();
    std::map<PhasePtr, PhasePtr> findStrongRelations(const std::vector<PhaseEdge>& edges);
    std::vector<PhaseNode> createUniqueNodes(const std::map<PhasePtr, PhasePtr>& rels);

    std::set<PhasePtr> _phases;
    std::map<std::string, PhasePtr> _nameMap;
};

PhaseGraph::PhaseGraph(std::set<std::shared_ptr<Phase>>& phases) : _phases(phases) {
    for (PhasePtr phase : _phases) {
        _nameMap[phase->getName()] = phase;
    }
}

std::vector<std::shared_ptr<Phase>> PhaseGraph::sort() {
    const std::vector<PhaseEdge>& edges(createEdges());
    const std::map<PhasePtr, PhasePtr>& rels(findStrongRelations(edges));
    const std::vector<PhaseNode>& nodes(createUniqueNodes(rels));

    for (const PhaseNode& node : nodes) {
        std::cout << "{ ";
        for (const PhasePtr& phase : node.phases) {
            std::cout << phase->getName() << " ";
        }
        std::cout << "}" << std::endl;
    }

    return {};
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

std::vector<PhaseEdge> PhaseGraph::createEdges() {
    std::vector<PhaseEdge> edges;

    for (PhasePtr phase : _phases) {
        if (PhasePtr rightAfter = phaseForName(phase->runsRightAfter())) {
            edges.push_back(PhaseEdge(rightAfter, phase, true));
        }
        if (PhasePtr rightBefore = phaseForName(phase->runsRightBefore())) {
            edges.push_back(PhaseEdge(phase, rightBefore, true));
        }

        for (const std::string& name : phase->runsAfter()) {
            if (PhasePtr after = phaseForName(name)) {
                edges.push_back(PhaseEdge(after, phase, false));
            }
        }

        for (const std::string& name  : phase->runsBefore()) {
            if (PhasePtr before = phaseForName(name)) {
                edges.push_back(PhaseEdge(phase, before, false));
            }
        }
    }

    return edges;
}

std::map<PhasePtr, PhasePtr> PhaseGraph::findStrongRelations(const std::vector<PhaseEdge>& edges) {
    std::map<PhasePtr, PhasePtr> rels;

    for (const PhaseEdge& edge : edges) {
        if (edge.isHard) {
            if (!rels.insert(std::make_pair(edge.from, edge.to)).second) {
                throw PhaseGraphResolutionError("Several hard dependencies were found for one node");
            }
        }
    }

    return rels;
}

PhasePtr find(std::map<PhasePtr, PhasePtr>& parents, const PhasePtr node) {
    auto it = parents.find(node);

    if (it != parents.end()) {
        if (it->second == node) {
            return node;
        } else {
            return find(parents, it->second);
        }
    } else {
        parents[node] = node;
        return node;
    }
}

void mergeSets(std::map<PhasePtr, PhasePtr>& parents, const PhasePtr first, const PhasePtr second) {
    const PhasePtr firstParent = find(parents, first);
    const PhasePtr secondParent = find(parents, second);
    parents[secondParent] = firstParent;
}

std::vector<PhaseNode> PhaseGraph::createUniqueNodes(const std::map<PhasePtr, PhasePtr>& rels) {
    std::map<PhasePtr, PhasePtr> parents;

    for (const auto& rel : rels) {
        mergeSets(parents, rel.first, rel.second);
    }

    std::map<PhasePtr, size_t> indexes;
    std::vector<PhaseNode> uniqs;

    for (const auto& rel : parents) {
        PhasePtr child = rel.first;
        PhasePtr parent = find(parents, rel.first);

        auto itParent = indexes.find(parent);

        if (itParent != indexes.end()) {
            uniqs[itParent->second].phases.push_back(child);
        } else {
            uniqs.push_back({});
            uniqs.back().phases.push_back(child);

            indexes[parent] = uniqs.size() - 1;
        }
    }

    return uniqs;
}

std::vector<PhasePtr> sortPhases(std::set<std::shared_ptr<Phase>>& phases) {
    PhaseGraph graph(phases);
    graph.sort();
    return {};
}

PhaseGraphResolutionError::PhaseGraphResolutionError(const std::string& err) : std::runtime_error(err) {

}

PhaseGraphResolutionError::~PhaseGraphResolutionError() {

}

}
