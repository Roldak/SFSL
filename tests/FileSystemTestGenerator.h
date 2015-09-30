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

#include "TestRunner.h"

namespace sfsl {

namespace test {

class FileSystemTestGenerator final {
public:
    FileSystemTestGenerator(const std::string& path);
    ~FileSystemTestGenerator();

    TestRunner* findAndGenerate();

private:

    const std::string& _path;
};

}

}

#endif
