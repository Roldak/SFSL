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
    std::vector<PhaseNode> createUniqueNodes(const std::map<PhasePtr, PhasePtr>& rels, const std::vector<PhaseEdge>& edges);

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
    const std::vector<PhaseNode>& nodes(createUniqueNodes(rels, edges));

    for (const PhaseNode& node : nodes) {
        std::cout << &node << " { ";
        for (const PhasePtr& phase : node.phases) {
            std::cout << phase->getName() << " ";
        }
        std::cout << "} -> { ";

        for (const PhaseNode* afters : node.edges) {
            std::cout << afters << " ";
        }

        std::cout << " }" << std::endl;
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

std::vector<PhaseNode> PhaseGraph::createUniqueNodes(const std::map<PhasePtr, PhasePtr>& rels, const std::vector<PhaseEdge>& edges) {
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

        std::cout << child->getName() << " -> " << parents[child]->getName() << " ; " <<depth << std::endl;

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

            from.edges.push_back(&to);
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
