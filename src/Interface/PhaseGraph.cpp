//
//  PhaseGraph.cpp
//  SFSL
//
//  Created by Romain Beguet on 12.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include <map>
#include <algorithm>
#include "PhaseGraph.h"

namespace sfsl {

typedef std::shared_ptr<Phase> PhasePtr;

struct PhaseNode final {

    enum MARK_TYPE { UNMARKED, TEMPORARY, PERMANENT };

    PhaseNode() : mark(UNMARKED) { }
    ~PhaseNode() { }

    bool isUnmarked() const {
        return mark == UNMARKED;
    }

    bool isTemporarilyMarked() const {
        return mark == TEMPORARY;
    }

    bool isPermanentlyMarked() const {
        return mark == PERMANENT;
    }

    void markTemporarily() {
        mark = TEMPORARY;
    }

    void markPermanently() {
        mark = PERMANENT;
    }

    std::vector<PhasePtr> phases;
    std::vector<PhaseNode*> edges;
    MARK_TYPE mark;
};

struct PhaseEdge final {
    PhaseEdge(PhasePtr from, PhasePtr to, bool isHard) : from(from), to(to), isHard(isHard) { }
    ~PhaseEdge() { }

    PhasePtr from;
    PhasePtr to;
    bool isHard;
};

PhasePtr phaseForName(const std::map<std::string, PhasePtr>& nameMap, const std::string& phaseName) {
    if (!phaseName.empty()) {
        auto it = nameMap.find(phaseName);
        if (it != nameMap.end()) {
            return it->second;
        } else {
            throw PhaseGraphResolutionError("Could not find any phase called '" + phaseName + "'");
        }
    }
    return PhasePtr(nullptr);
}

std::vector<PhaseEdge> createEdges(std::set<PhasePtr> phases) {
    std::map<std::string, PhasePtr> nameMap;
    std::vector<PhaseEdge> edges;

    for (PhasePtr phase : phases) {
        nameMap[phase->getName()] = phase;
    }

    for (PhasePtr phase : phases) {
        if (PhasePtr rightAfter = phaseForName(nameMap, phase->runsRightAfter())) {
            edges.push_back(PhaseEdge(rightAfter, phase, true));
        }
        if (PhasePtr rightBefore = phaseForName(nameMap, phase->runsRightBefore())) {
            edges.push_back(PhaseEdge(phase, rightBefore, true));
        }

        for (const std::string& name : phase->runsAfter()) {
            if (PhasePtr after = phaseForName(nameMap, name)) {
                edges.push_back(PhaseEdge(after, phase, false));
            }
        }

        for (const std::string& name  : phase->runsBefore()) {
            if (PhasePtr before = phaseForName(nameMap, name)) {
                edges.push_back(PhaseEdge(phase, before, false));
            }
        }
    }

    return edges;
}

std::map<PhasePtr, PhasePtr> findStrongRelations(const std::vector<PhaseEdge>& edges) {
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

PhasePtr findHelper(std::map<PhasePtr, PhasePtr>& parents, const PhasePtr node, std::set<PhasePtr>& visited, size_t* depth) {
    if (!visited.insert(node).second) {
        throw PhaseGraphResolutionError("A cyclic dependency was found in the phase graph");
    }

    auto it = parents.find(node);

    if (it->second == node) {
        return node;
    } else {
        if (depth) (*depth)++;
        return findHelper(parents, it->second, visited, depth);
    }
}

PhasePtr find(std::map<PhasePtr, PhasePtr>& parents, const PhasePtr node, size_t* depth = nullptr) {
    std::set<PhasePtr> visited;
    return findHelper(parents, node, visited, depth);
}

std::vector<PhaseNode> createUniqueNodes(const std::map<PhasePtr, PhasePtr>& rels, const std::vector<PhaseEdge>& edges) {
    std::map<PhasePtr, PhasePtr> parents;

    for (const PhaseEdge& edge : edges) {
        parents[edge.from] = edge.from;
        parents[edge.to] = edge.to;
    }

    for (const auto& rel : rels) {
        parents[rel.second] = rel.first;
    }

    std::map<PhasePtr, size_t> indexes;
    std::vector<PhaseNode> uniqs;

    for (const auto& hardrel : parents) {
        size_t depth = 0;

        PhasePtr child = hardrel.first;
        PhasePtr parent = find(parents, hardrel.first, &depth);

        auto itParent = indexes.find(parent);

        if (itParent == indexes.end()) {
            uniqs.push_back({});
            itParent = indexes.insert(std::make_pair(parent, uniqs.size() - 1)).first;
        }

        std::vector<PhasePtr>& phases = uniqs[itParent->second].phases;
        if (depth >= phases.size()) {
            phases.resize(depth + 1);
        }

        phases[depth] = child;
    }

    for (const PhaseEdge& edge : edges) {
        if (!edge.isHard) {
            PhaseNode& from = uniqs[indexes[find(parents, edge.from)]];
            PhaseNode& to = uniqs[indexes[find(parents, edge.to)]];

            to.edges.push_back(&from);
        }
    }

    return uniqs;
}

void topologicalSortDFSvisit(std::vector<PhasePtr>& ordered, PhaseNode* node) {
    if (node->isTemporarilyMarked()) {
        throw PhaseGraphResolutionError("A cyclic dependency was found");
    }

    if (node->isUnmarked()) {
        node->markTemporarily();

        for (PhaseNode* neighbor : node->edges) {
            topologicalSortDFSvisit(ordered, neighbor);
        }

        node->markPermanently();

        for (PhasePtr phase : node->phases) {
            ordered.push_back(phase);
        }
    }
}

std::vector<PhasePtr> topologicalSort(std::vector<PhaseNode>& nodes) {
    std::vector<PhasePtr> ordered;

    for (PhaseNode& node : nodes) {
        if (!node.isUnmarked()) {
            continue;
        }

        topologicalSortDFSvisit(ordered, &node);
    }

    return ordered;
}

std::vector<PhasePtr> sortPhases(std::set<std::shared_ptr<Phase>>& phases) {
    const std::vector<PhaseEdge>& edges(createEdges(phases));
    const std::map<PhasePtr, PhasePtr>& rels(findStrongRelations(edges));

    std::vector<PhaseNode> nodes(createUniqueNodes(rels, edges));
    std::vector<PhasePtr> sortedPhases(topologicalSort(nodes));

    return sortedPhases;
}

PhaseGraphResolutionError::PhaseGraphResolutionError(const std::string& err) : std::runtime_error(err) {

}

PhaseGraphResolutionError::~PhaseGraphResolutionError() {

}

}
