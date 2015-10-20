//
//  AbstractScopeReporter.h
//  SFSL
//
//  Created by Romain Beguet on 20.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__AbstractScopeReporter__
#define __SFSL__AbstractScopeReporter__

#include "Compiler/Common/Positionnable.h"

namespace sfsl {

namespace complete {

class AbstractScopeReporter {
public:
    virtual ~AbstractScopeReporter();
    virtual void reportScope(common::Positionnable* pos, const std::vector<size_t>& scopeId) = 0;
};

class StandartScopeReporter : public AbstractScopeReporter {
public:
    StandartScopeReporter(std::ostream& stream);
    virtual ~StandartScopeReporter();

    virtual void reportScope(common::Positionnable* pos, const std::vector<size_t>& scopeId);

    static StandartScopeReporter* CoutReporter;
    static StandartScopeReporter* CerrReporter;

private:

    std::ostream& _stream;
};

}

}

#endif
