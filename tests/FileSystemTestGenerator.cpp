//
//  FileSystemTestGenerator.cpp
//  SFSL
//
//  Created by Romain Beguet on 30.09.2015.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "FileSystemTestGenerator.h"

#include "dirent.h"

namespace sfsl {

namespace test {

FileSystemTestGenerator::FileSystemTestGenerator(const std::__cxx11::string& path) : _path(path) {

}

FileSystemTestGenerator::~FileSystemTestGenerator() {

}

TestRunner* FileSystemTestGenerator::findAndGenerate() {

}

}

}
