//
//  CapturesTest.h
//  SFSL
//
//  Created by Romain Beguet on 11.11.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__CapturesTestTest__
#define __SFSL__CapturesTestTest__

#include <iostream>

#include "CompilationTest.h"

namespace sfsl {

namespace test {

class CapturesTest : public CompilationTest {
public:
    CapturesTest(const std::string& name, const std::string& source, bool shouldCompile);

    virtual ~CapturesTest();
};

}

}

#endif
