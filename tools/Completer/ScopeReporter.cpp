//
//  AbstractScopeReporter.cpp
//  SFSL
//
//  Created by Romain Beguet on 20.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "ScopeReporter.h"

namespace sfsl {

namespace complete {

StandartScopeReporter* StandartScopeReporter::CoutReporter = new StandartScopeReporter(std::cout);
StandartScopeReporter* StandartScopeReporter::CerrReporter = new StandartScopeReporter(std::cerr);

AbstractScopeReporter::~AbstractScopeReporter() {

}

StandartScopeReporter::StandartScopeReporter(std::ostream& stream) : _stream(stream) {

}

StandartScopeReporter::~StandartScopeReporter() {

}

void StandartScopeReporter::reportScope(common::Positionnable* pos, const std::vector<size_t>& scopeId) {
    _stream << pos->getStartPosition() << ":" << pos->getEndPosition() << ":" << scopeId.front();
    for (size_t i = 1; i < scopeId.size(); ++i) {
        _stream << "." << scopeId[i];
    }
    _stream << std::endl;
}

}

}
