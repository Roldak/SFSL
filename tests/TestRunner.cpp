//
//  TestRunner.cpp
//  SFSL
//
//  Created by Romain Beguet on 30.09.2015.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "TestRunner.h"

namespace sfsl {

namespace test {

TestRunner::TestRunner(const std::string& name, const std::vector<TestSuite*>& testSuites)
    : _name(name), _testSuites(testSuites) {

}

TestRunner::~TestRunner() {
    for (TestSuite* suite : _testSuites) {
        delete suite;
    }
}

bool TestRunner::run(AbstractTestLogger& logger) const {
    size_t successCount = 0;

    logger.testRunnerStart(_name);

    for (TestSuite* suite : _testSuites) {
        if (suite->run(logger)) {
            ++successCount;
        }
    }

    logger.testRunnerEnd(successCount, _testSuites.size());

    return successCount == _testSuites.size();
}

}

}
