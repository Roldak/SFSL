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

class SymbolAssertionsPhase : public Phase {
public:
    SymbolAssertionsPhase() : Phase("SymbolAssertions", "Checks symbol assertions for tests") { }
    virtual ~SymbolAssertionsPhase() { }

    virtual std::string runsRightAfter() const { return "NameAnalysis"; }

    virtual bool run(PhaseContext& pctx) {
        ast::Program* prog = pctx.require<ast::Program>("prog");
        CompCtx_Ptr ctx = *pctx.require<CompCtx_Ptr>("ctx");

        SymbolAssertionsChecker sac(ctx);
        prog->onVisit(&sac);

        return ctx->reporter().getErrorCount() == 0;
    }
};

SymbolicTest::SymbolicTest(const std::string& name, const std::string& source)
    : AbstractTest(name), _source(source), _ppl(Pipeline::createDefault()) {
    _ppl.insert(Phase::StopRightAfter("SymbolAssertions"));
    _ppl.insert(std::shared_ptr<Phase>(new SymbolAssertionsPhase()));
}

SymbolicTest::~SymbolicTest() {

}

bool SymbolicTest::run(AbstractTestLogger& logger) {
    Compiler cmp(CompilerConfig(StandartReporter::EmptyReporter, 2048));

    try {
        ProgramBuilder builder = cmp.parse(_name, _source);
        if (!builder) {
            logger.result(_name, false, std::string("Fatal: failed to parse the program"));
        } else {
            try {
                buildSTDModules(cmp, builder);
                ErrorCountCollector errcount;
                cmp.compile(builder, errcount, _ppl);
                logger.result(_name, (errcount.get() == 0));
                return true;
            } catch (const CompileError& err) {
                logger.result(_name, false, std::string("Fatal: ") + err.what());
            }
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
