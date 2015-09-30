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

CompilationTest::CompilationTest(const std::string& name, const std::string& source, bool shouldCompile)
    : AbstractTest(name), _source(source), _shouldCompile(shouldCompile) {

}

CompilationTest::~CompilationTest() {

}

bool CompilationTest::run(AbstractTestLogger& logger) {
    bool success;

    try {
        Compiler cmp(CompilerConfig(2048));
        ProgramBuilder builder = cmp.parse(_name, _source);
        buildSTDModules(cmp, builder);
        success = (!cmp.compile(builder).empty()) == _shouldCompile;
        logger.result(_name, success);
    } catch (const CompileError& err) {
        success = !_shouldCompile;
        logger.result(_name, success, std::string("Fatal: ") + err.what());
    }

    return success;
}

void CompilationTest::buildSTDModules(Compiler& cmp, ProgramBuilder builder) {
    Module slang = builder.openModule("sfsl").openModule("lang");
    slang.typeDef("unit", cmp.classBuilder("unit").build());
    slang.typeDef("bool", cmp.classBuilder("bool").build());
    slang.typeDef("int", cmp.classBuilder("int").build());
    slang.typeDef("real", cmp.classBuilder("real").build());
    slang.typeDef("string", cmp.classBuilder("string").build());
}

}

}
