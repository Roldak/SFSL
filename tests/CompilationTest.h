//
//  CompilationTest.h
//  SFSL
//
//  Created by Romain Beguet on 30.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__CompilationTest__
#define __SFSL__CompilationTest__

#include <iostream>

#include "AbstractTest.h"

namespace sfsl {

namespace test {

class CompilationTest : public AbstractTest {
public:
    CompilationTest();
    virtual ~CompilationTest();
};

}

}

#endif
