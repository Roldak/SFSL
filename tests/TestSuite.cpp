//
//  TestSuite.cpp
//  SFSL
//
//  Created by Romain Beguet on 30.09.2015.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "TestSuite.h"

namespace sfsl {

namespace test {

// TEST SUITE

TestSuite::TestSuite(const std::string& name, const std::vector<AbstractTest*> tests)
    : _name(name), _tests(tests) {

}

TestSuite::~TestSuite() {
    for (AbstractTest* test : _tests) {
        delete test;
    }
}

bool TestSuite::run(AbstractTestLogger& logger) const {
    size_t successCount = 0;

    logger.testSuiteStart(_name);

    for (AbstractTest* test : _tests) {
        if (test->run(logger)) {
            ++successCount;
        }
    }

    logger.testSuiteEnd(successCount, _tests.size());

    return successCount == _tests.size();
}

// TEST SUITE BUILDER

void TestSuiteBuilder::setName(const std::__cxx11::string& name) {
    _name = name;
}

void TestSuiteBuilder::addTest(AbstractTest* test) {
    _tests.push_back(test);
}

TestSuite* TestSuiteBuilder::build() const {
    return new TestSuite(_name, _tests);
}

}

}
