//
//  FileSystemTestGenerator.cpp
//  SFSL
//
//  Created by Romain Beguet on 30.09.2015.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include <fstream>
#include <sstream>
#include <algorithm>

#include "FileSystemTestGenerator.h"
#include "CompilationTest.h"
#include "SymbolicTest.h"
#include "CapturesTest.h"

namespace sfsl {

namespace test {

FileSystemTestGenerator::FileSystemTestGenerator(const std::string& path) : _path(path) {

}

FileSystemTestGenerator::~FileSystemTestGenerator() {

}

bool isValidEntryName(const std::string& name) {
    return name != "." && name != "..";
}

TestRunner* FileSystemTestGenerator::findAndGenerate() {
    std::vector<TestSuite*> testSuites;

    if (DIR* root = opendir(_path.c_str())) {
        while (dirent* ent = readdir(root)) {
            std::string entryName(ent->d_name, ent->d_namlen);

            if (isValidEntryName(entryName)) {
                std::string subdirPath = _path + "/" + entryName;

                if (DIR* subdir = opendir(subdirPath.c_str())) {
                    TestSuiteBuilder builder(entryName);
                    buildTestSuite(builder, subdirPath, subdir);
                    testSuites.push_back(builder.build());
                    closedir(subdir);
                }
            }
        }

        closedir(root);
    }

    return new TestRunner("FileSystemTestGenerator", testSuites);
}

FileSystemTestGenerator::TEST_TYPE FileSystemTestGenerator::typeFromName(const std::string& name) {
    if (name == "MustCompile") {
        return MUST_COMPILE;
    } else if (name == "MustNotCompile") {
        return MUST_NOT_COMPILE;
    } else {
        return UNKNOWN_TEST_TYPE;
    }
}

void FileSystemTestGenerator::buildTestSuite(TestSuiteBuilder& builder, const std::string& path, DIR* dir) {
    while (dirent* ent = readdir(dir)) {
        std::string entryName(ent->d_name, ent->d_namlen);
        TEST_TYPE type = typeFromName(entryName);

        if (isValidEntryName(entryName) && type != UNKNOWN_TEST_TYPE) {
            std::string subdirPath = path + "/" + entryName;

            if (DIR* subdir = opendir(subdirPath.c_str())) {
                createTestsForType(builder, type, subdirPath, subdir);
                closedir(subdir);
            }
        }
    }
}

void FileSystemTestGenerator::createTestsForType(TestSuiteBuilder& builder, FileSystemTestGenerator::TEST_TYPE type, const std::string& path, DIR* dir) {
    while (dirent* ent = readdir(dir)) {
        std::string testPath(ent->d_name, ent->d_namlen);

        if (isValidEntryName(testPath)) {
            std::string testName = testNameFromTestPath(testPath);

            std::ifstream f(path + "/" + testPath);
            std::stringstream buffer;
            buffer << f.rdbuf();

            std::string source = buffer.str();

            if (builder.getName() == "NameAnalysis" && type == MUST_COMPILE) {
                builder.addTest(new SymbolicTest(testName, source));
            } else if (builder.getName() == "Captures") {
                builder.addTest(new CapturesTest(testName, source, type == MUST_COMPILE));
            } else {
                builder.addTest(new CompilationTest(testName, source, type == MUST_COMPILE, builder.getName()));
            }
        }
    }
}

std::string FileSystemTestGenerator::testNameFromTestPath(const std::string& path) {
    std::string copy;
    size_t indexOfDot = path.find('.');

    if (indexOfDot != std::string::npos) {
        copy = path.substr(0, indexOfDot);
    } else {
        copy = path;
    }

    std::replace(copy.begin(), copy.end(), '_', ' ');
    return copy;
}

}

}
