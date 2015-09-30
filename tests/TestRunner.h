//
//  TestRunner.h
//  SFSL
//
//  Created by Romain Beguet on 30.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__TestRunner__
#define __SFSL__TestRunner__

#include <iostream>

#include "TestSuite.h"

namespace sfsl {

namespace test {

class TestRunner final {
public:
    ~TestRunner();

    bool run(AbstractTestLogger& logger) const;

private:
    friend class TestGenerator;

    TestRunner(const std::string& name, const std::vector<TestSuite*>& testSuites);

    const std::string _name;
    const std::vector<TestSuite*> _testSuites;
};

}

}

#endif
