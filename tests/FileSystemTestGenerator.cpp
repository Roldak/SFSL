//
//  FileSystemTestGenerator.cpp
//  SFSL
//
//  Created by Romain Beguet on 30.09.2015.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include <fstream>
#include <algorithm>

#include "FileSystemTestGenerator.h"
#include "CompilationTest.h"

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
            std::string source;

            std::ifstream f(path + "/" + testPath);
            while (f.good()) {
                source += f.get();
            }

            switch (type) {
            case MUST_COMPILE:
                builder.addTest(new CompilationTest(testName, source, true, builder.getName()));
                break;
            case MUST_NOT_COMPILE:
                builder.addTest(new CompilationTest(testName, source, false, builder.getName()));
                break;
            default:
                break;
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
