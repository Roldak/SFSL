//
//  CapturesTest.cpp
//  SFSL
//
//  Created by Romain Beguet on 11.11.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#include "CapturesTest.h"
#include "Visitors/CapturesChecker.h"
#include "Compiler/Backend/AST2BAST/PreTransform.h"

namespace sfsl {

namespace test {

class CapturesCheckPhase : public Phase {
public:
    CapturesCheckPhase() : Phase("CapturesCheck", "Performs some checks on captures with user-written annotations") { }
    virtual ~CapturesCheckPhase() { }

    virtual std::string runsRightAfter() const override { return {"UsageAnalysis"}; }

    virtual bool run(PhaseContext& pctx) {
        ast::Program* prog = pctx.require<ast::Program>("prog");
        CompCtx_Ptr ctx = *pctx.require<CompCtx_Ptr>("ctx");

        ast::PreTransformAnalysis ptanalysis(ctx);
        CapturesChecker cc(ctx);

        prog->onVisit(&ptanalysis);
        prog->onVisit(&cc);

        return ctx->reporter().getErrorCount() == 0;
    }
};

CapturesTest::CapturesTest(const std::string& name, const std::string& source, bool shouldCompile)
    : CompilationTest(name, source, shouldCompile, "CapturesCheck") {
    _ppl.insert(std::shared_ptr<Phase>(new CapturesCheckPhase()));
}

CapturesTest::~CapturesTest() {

}

}

}
