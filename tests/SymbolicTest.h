//
//  SymbolicTest.h
//  SFSL
//
//  Created by Romain Beguet on 02.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__SymbolicTest__
#define __SFSL__SymbolicTest__

#include <iostream>

#include "sfsl.h"
#include "AbstractTest.h"

namespace sfsl {

namespace test {

class SymbolicTest : public AbstractTest {
public:
    SymbolicTest(const std::string& name, const std::string& source);
    virtual ~SymbolicTest();

    virtual bool run(AbstractTestLogger& logger) override;

private:

    const std::string _source;
    Pipeline _ppl;
};

}

}

#endif
