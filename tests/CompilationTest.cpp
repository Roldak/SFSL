//
//  CompilationTest.cpp
//  SFSL
//
//  Created by Romain Beguet on 30.09.2015.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "CompilationTest.h"

namespace sfsl {

namespace test {

CompilationTest::CompilationTest(const std::string& name, Compiler& cmp, const std::string& source, bool shouldCompile)
    : AbstractTest(name), _cmp(cmp), _source(source), _shouldCompile(shouldCompile) {

}

CompilationTest::~CompilationTest() {

}

bool CompilationTest::run(AbstractTestLogger& logger) {
    bool success;

    try {
        ProgramBuilder builder = _cmp.parse(_name, _source);
        buildSTDModules(builder);
        success = (!_cmp.compile(builder).empty()) == _shouldCompile;
        logger.result(_name, success);
    } catch (const CompileError& err) {
        success = !_shouldCompile;
        logger.result(_name, success, std::string("Fatal: ") + err.what());
    }

    return success;
}

void CompilationTest::buildSTDModules(ProgramBuilder builder) {
    Module slang = builder.openModule("sfsl").openModule("lang");
    slang.typeDef("unit", _cmp.classBuilder("unit").build());
    slang.typeDef("bool", _cmp.classBuilder("bool").build());
    slang.typeDef("int", _cmp.classBuilder("int").build());
    slang.typeDef("real", _cmp.classBuilder("real").build());
    slang.typeDef("string", _cmp.classBuilder("string").build());
}

}

}
