//
//  AbstractTestLogger.h
//  SFSL
//
//  Created by Romain Beguet on 30.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__AbstractTestLogger__
#define __SFSL__AbstractTestLogger__

#include <iostream>

namespace sfsl {

namespace test {

class AbstractTestLogger {
public:
    virtual ~AbstractTestLogger();

    virtual void testRunnerStart(const std::string& testRunnerName) = 0;
    virtual void testSuiteStart(const std::string& testSuiteName) = 0;
    virtual void result(const std::string& testName, bool success, const std::string& note = "") = 0;
    virtual void testSuiteEnd(size_t passedTests, size_t totalTests) = 0;
    virtual void testRunnerEnd(size_t passedSuites, size_t totalSuites) = 0;

private:
};

}

}

#endif
