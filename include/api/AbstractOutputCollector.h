//
//  AbstractOutputCollector.h
//  SFSL
//
//  Created by Romain Beguet on 16.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__API_AbstractOutputCollector__
#define __SFSL__API_AbstractOutputCollector__

#include <iostream>
#include <map>
#include "PhaseContext.h"
#include "SetVisibilities.h"

namespace sfsl {

class SFSL_API_PUBLIC AbstractOutputCollector {
public:
    AbstractOutputCollector();
    virtual ~AbstractOutputCollector();

    virtual void collect(PhaseContext& pctx) = 0;
};

}

#endif
