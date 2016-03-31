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

CompilationTest::CompilationTest(const std::string& name, const std::string& source, bool shouldCompile, const std::string& lastPhase)
    : AbstractTest(name), _source(source), _shouldCompile(shouldCompile), _lastPhase(lastPhase), _ppl(Pipeline::createDefault()) {
    _ppl.insert(Phase::StopRightAfter(lastPhase));
}

CompilationTest::~CompilationTest() {

}

bool CompilationTest::run(AbstractTestLogger& logger) {
    bool success;

    Compiler cmp(CompilerConfig(StandartReporter::EmptyReporter, StandartPrimitiveNamer::DefaultPrimitiveNamer, 2048));
    try {
        ProgramBuilder builder = cmp.parse(_name, _source);
        if (!builder) {
            logger.result(_name, false, std::string("Fatal: failed to parse the program"));
            success = false;
        } else {
            try {
                cmp.loadPlugin("libstdlib-d");
                ErrorCountCollector errcount;
                cmp.compile(builder, errcount, _ppl);
                success = ((errcount.get() == 0) == _shouldCompile);
                logger.result(_name, success);
            } catch (const CompileError& err) {
                success = !_shouldCompile;
                logger.result(_name, success, std::string("Fatal: ") + err.what());
            }
        }
    } catch (const CompileError& err) {
        logger.result(_name, false, std::string("Fatal: ") + err.what());
        success = false;
    }

    return success;
}

}

}
