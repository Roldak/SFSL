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
#include "Compiler/Backend/AST2BAST/UserDataAssignment.h"
#include "Compiler/Backend/AST2BAST/AST2BAST.h"
#include "Compiler/Backend/BAST/Visitors/BASTPrinter.h"
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
        common::AbstractPrimitiveNamer* namer = pctx.require<common::AbstractPrimitiveNamer>("namer");

        sym::SymbolResolver res(prog, namer, ctx);

        ast::TopLevelTypeChecking topleveltypecheck(ctx, *namer, res);
        ast::TypeChecking typeCheck(ctx, *namer, res);

        prog->onVisit(&topleveltypecheck);
        prog->onVisit(&typeCheck);

        return ctx->reporter().getErrorCount() == 0;
    }
};

class PreTransformPhase : public Phase {
public:
    PreTransformPhase() : Phase("PreTransform", "Assigns useful informations to node and performs last minute operations") { }
    virtual ~PreTransformPhase() { }

    virtual std::vector<std::string> runsAfter() const { return {"TypeChecking"}; }

    virtual bool run(PhaseContext& pctx) {
        ast::Program* prog = pctx.require<ast::Program>("prog");
        CompCtx_Ptr ctx = *pctx.require<CompCtx_Ptr>("ctx");

        ast::UserDataAssignment uda(ctx);
        ast::AnnotationUsageWarner auw(ctx);

        prog->onVisit(&uda);
        prog->onVisit(&auw);

        return ctx->reporter().getErrorCount() == 0;
    }
};

class AST2BASTPhase : public Phase {
public:
    AST2BASTPhase() : Phase("AST2BAST", "Transforms the frontent AST into the backend AST") { }
    virtual ~AST2BASTPhase() { }

    virtual std::vector<std::string> runsAfter() const { return {"PreTransform"}; }

    virtual bool run(PhaseContext& pctx) {
        ast::Program* prog = pctx.require<ast::Program>("prog");
        CompCtx_Ptr ctx = *pctx.require<CompCtx_Ptr>("ctx");

        bast::AST2BAST a2b(ctx);
        bast::BASTSimplifier simplifier(ctx);
        bast::BASTPrinter printer(ctx, std::cout);

        bast::Program* bprog = a2b.transform(prog);
        //bprog->onVisit(&simplifier);
        bprog->onVisit(&printer);

        pctx.output("bprog", bprog);

        return ctx->reporter().getErrorCount() == 0;
    }
};

class CodeGenPhase : public Phase {
public:
    CodeGenPhase() : Phase("CodeGen", "Emits sfsl bytecode from the backend abstract syntax tree"), _out(nullptr) { }
    virtual ~CodeGenPhase() {
        cleanup();
    }

    virtual std::vector<std::string> runsAfter() const { return {"AST2BAST"}; }

    virtual bool run(PhaseContext& pctx) {
        ast::Program* prog = pctx.require<ast::Program>("prog");
        CompCtx_Ptr ctx = *pctx.require<CompCtx_Ptr>("ctx");

        cleanup();

        _out = new out::LinkedListOutput<bc::BCInstruction*>(ctx);

        bc::DefaultBytecodeGenerator gen(ctx, *_out);
        prog->onVisit(&gen);

        pctx.output("out", _out);

        return ctx->reporter().getErrorCount() == 0;
    }

private:

    void cleanup() {
        if (_out) {
            delete _out;
        }
    }

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
    ppl.insert(std::shared_ptr<Phase>(new PreTransformPhase));
    ppl.insert(std::shared_ptr<Phase>(new AST2BASTPhase));
    ppl.insert(std::shared_ptr<Phase>(new CodeGenPhase));

    return ppl;
}

}
