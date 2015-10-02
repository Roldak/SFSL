//
//  SymbolicTest.cpp
//  SFSL
//
//  Created by Romain Beguet on 02.10.2015.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "SymbolicTest.h"
#include "Visitors/SymbolAssertionsChecker.h"

namespace sfsl {

namespace test {

SymbolicTest::SymbolicTest(const std::string& name, const std::string& source)
    : AbstractTest(name), _source(source) {

}

SymbolicTest::~SymbolicTest() {

}

bool SymbolicTest::run(AbstractTestLogger& logger) {
    Compiler cmp(CompilerConfig(StandartReporter::EmptyReporter, 2048));

    try {
        ProgramBuilder builder = cmp.parse(_name, _source);
        try {
            buildSTDModules(cmp, builder);
            std::vector<std::string> res = cmp.compile(builder);
            logger.result(_name, !res.empty());
            return true;
        } catch (const CompileError& err) {
            logger.result(_name, false, std::string("Fatal: ") + err.what());
        }

    } catch (const CompileError& err) {
        logger.result(_name, false, std::string("Fatal: ") + err.what());
    }

    return false;
}

void SymbolicTest::buildSTDModules(Compiler& cmp, ProgramBuilder builder) {
    Module slang = builder.openModule("sfsl").openModule("lang");
    slang.typeDef("unit", cmp.classBuilder("unit").build());
    slang.typeDef("bool", cmp.classBuilder("bool").build());
    slang.typeDef("int", cmp.classBuilder("int").build());
    slang.typeDef("real", cmp.classBuilder("real").build());
    slang.typeDef("string", cmp.classBuilder("string").build());

    builder.openModule("test").externDef(ASSERT_SAME_SYM, cmp.parseType("(sfsl.lang.string, sfsl.lang.unit)->sfsl.lang.unit"));
}

}

}
