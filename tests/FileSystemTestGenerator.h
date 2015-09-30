//
//  FileSystemTestGenerator.h
//  SFSL
//
//  Created by Romain Beguet on 30.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__FileSystemTestGenerator__
#define __SFSL__FileSystemTestGenerator__

#include <iostream>
#include "dirent.h"

#include "TestRunner.h"

namespace sfsl {

namespace test {

class FileSystemTestGenerator final {
public:

    enum TEST_TYPE {
        MUST_COMPILE,
        MUST_NOT_COMPILE,
        UNKNOWN_TEST_TYPE
    };

    FileSystemTestGenerator(const std::string& path);
    ~FileSystemTestGenerator();

    TestRunner* findAndGenerate();

private:

    static TEST_TYPE typeFromName(const std::string& name);

    void buildTestSuite(TestSuiteBuilder& builder, const std::string& path, DIR* dir);
    void createTestsForType(TestSuiteBuilder& builder, TEST_TYPE type, DIR* dir);

    const std::string _path;
};

}

}

#endif
