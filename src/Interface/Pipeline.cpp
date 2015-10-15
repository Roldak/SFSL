//
//  Pipeline.cpp
//  SFSL
//
//  Created by Romain Beguet on 04.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "api/Pipeline.h"
#include "api/Phase.h"

#include "Compiler/Frontend/Analyser/NameAnalysis.h"
#include "Compiler/Frontend/Analyser/KindChecking.h"
#include "Compiler/Frontend/Analyser/TypeChecking.h"
#include "Compiler/Frontend/Symbols/SymbolResolver.h"
#include "Compiler/Backend/UserDataAssignment.h"
#include "Compiler/Backend/BytecodeGenerator.h"

namespace sfsl {

// PHASES

class NameAnalysisPhase : public Phase {
public:
    NameAnalysisPhase() : Phase("NameAnalysis", "Generates scopes, allocate symbols and assigns them to the symbolics") { }
    virtual ~NameAnalysisPhase() { }

    virtual bool run(PhaseContext& pctx) {
        ast::Program* prog = pctx.require<ast::Program>("prog");
        CompCtx_Ptr ctx = *pctx.require<CompCtx_Ptr>("ctx");

        ast::ScopeGeneration scopeGen(ctx);
        ast::TypeDependencyFixation typeDep(ctx);
        ast::SymbolAssignation symAssign(ctx);

        prog->onVisit(&scopeGen);
        prog->onVisit(&typeDep);
        prog->onVisit(&symAssign);

        return ctx->reporter().getErrorCount() == 0;
    }
};

class KindCheckingPhase : public Phase {
public:
    KindCheckingPhase() : Phase("KindChecking", "Assigns kinds to every kinded nodes, and reports any kind check errors") { }
    virtual ~KindCheckingPhase() { }

    virtual std::vector<std::string> runsAfter() const { return {"NameAnalysis"}; }

    virtual bool run(PhaseContext &pctx) {
        ast::Program* prog = pctx.require<ast::Program>("prog");
        CompCtx_Ptr ctx = *pctx.require<CompCtx_Ptr>("ctx");

        ast::KindChecking kindCheck(ctx);
        prog->onVisit(&kindCheck);

        return ctx->reporter().getErrorCount() == 0;
    }
};

class TypeCheckingPhase : public Phase {
public:
    TypeCheckingPhase() : Phase("TypeChecking", "Assigns types to every typed nodes, and reports any type check errors") { }
    virtual ~TypeCheckingPhase() { }

    virtual std::vector<std::string> runsAfter() const { return {"KindChecking"}; }

    virtual bool run(PhaseContext& pctx) {
        ast::Program* prog = pctx.require<ast::Program>("prog");
        CompCtx_Ptr ctx = *pctx.require<CompCtx_Ptr>("ctx");

        sym::SymbolResolver res(prog, ctx);
        res.setPredefClassesPath("sfsl.lang");

        ast::TopLevelTypeChecking topleveltypecheck(ctx, res);
        ast::TypeChecking typeCheck(ctx, res);

        prog->onVisit(&topleveltypecheck);
        prog->onVisit(&typeCheck);

        return ctx->reporter().getErrorCount() == 0;
    }
};

class CodeGenPhase : public Phase {
public:
    CodeGenPhase() : Phase("CodeGen", "Emits sfsl bytecode from the abstract syntax tree"), _out(nullptr) { }
    virtual ~CodeGenPhase() {
        if (_out) delete _out;
    }

    virtual std::vector<std::string> runsAfter() const { return {"TypeChecking"}; }

    virtual bool run(PhaseContext& pctx) {
        ast::Program* prog = pctx.require<ast::Program>("prog");
        CompCtx_Ptr ctx = *pctx.require<CompCtx_Ptr>("ctx");

        _out = new out::LinkedListOutput<bc::BCInstruction*>(ctx);
        bc::UserDataAssignment uda(ctx);
        bc::DefaultBytecodeGenerator gen(ctx, *_out);

        prog->onVisit(&uda);
        prog->onVisit(&gen);

        pctx.output("out", _out);

        return ctx->reporter().getErrorCount() == 0;
    }

private:

    out::LinkedListOutput<bc::BCInstruction*>* _out;
};

// PIPELINE

Pipeline::Pipeline() {

}

std::set<std::shared_ptr<Phase>> Pipeline::getPhases() const {
    return _phases;
}

Pipeline::~Pipeline() {

}

Pipeline& Pipeline::insert(std::shared_ptr<Phase> phase) {
    _phases.insert(phase);
    return *this;
}

Pipeline Pipeline::createEmpty() {
    return Pipeline();
}

Pipeline Pipeline::createDefault() {
    Pipeline ppl;

    ppl.insert(std::shared_ptr<Phase>(new NameAnalysisPhase));
    ppl.insert(std::shared_ptr<Phase>(new KindCheckingPhase));
    ppl.insert(std::shared_ptr<Phase>(new TypeCheckingPhase));
    ppl.insert(std::shared_ptr<Phase>(new CodeGenPhase));

    return ppl;
}

}
