//
//  AbstractTest.h
//  SFSL
//
//  Created by Romain Beguet on 30.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__AbstractTest__
#define __SFSL__AbstractTest__

#include <iostream>

#include "AbstractTestLogger.h"

namespace sfsl {

namespace test {

class AbstractTest {
public:
    AbstractTest(const std::string& name);
    virtual ~AbstractTest();

    virtual bool run(AbstractTestLogger& logger) = 0;

protected:

    const std::string _name;
};

}

}

#endif
