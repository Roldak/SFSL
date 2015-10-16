//
//  TestSuite.h
//  SFSL
//
//  Created by Romain Beguet on 30.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__TestSuite__
#define __SFSL__TestSuite__

#include <iostream>
#include <vector>

#include "AbstractTest.h"

namespace sfsl {

namespace test {

class TestSuite final {
public:
    ~TestSuite();

    bool run(AbstractTestLogger& logger) const;

private:
    friend class TestSuiteBuilder;

    TestSuite(const std::string& name, const std::vector<AbstractTest*> tests);

    const std::string _name;
    const std::vector<AbstractTest*> _tests;
};

class TestSuiteBuilder final {
public:
    TestSuiteBuilder(const std::string& name);
    ~TestSuiteBuilder();

    void addTest(AbstractTest* test);
    TestSuite* build() const;

    const std::string& getName() const;

private:

    std::string _name;
    std::vector<AbstractTest*> _tests;
};

}

}

#endif
