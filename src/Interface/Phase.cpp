//
//  Phase.cpp
//  SFSL
//
//  Created by Romain Beguet on 04.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "api/Phase.h"
#include "Compiler/Frontend/AST/Visitors/ASTPrinter.h"

namespace sfsl {

class PhasePrettyPrint : public Phase {
public:
    PhasePrettyPrint(std::ostream& ostream)
        : Phase("PrettyPrint", "Prints the AST in a pretty way."), _ostream(ostream) { }

    virtual ~PhasePrettyPrint() { }

    virtual std::string runsRightBefore() const { return "NameAnalysis"; }

    virtual bool run(PhaseContext& pctx) {
        ast::Program* prog = pctx.require<ast::Program>("prog");
        CompCtx_Ptr ctx = *pctx.require<CompCtx_Ptr>("ctx");

        ast::ASTPrinter printer(ctx, _ostream);
        prog->onVisit(&printer);

        return true;
    }

private:

    std::ostream& _ostream;
};

class PhaseStop : public Phase {
public:
    PhaseStop(bool after, const std::string& phase)
        : Phase("_StopAfter_", "Phase that makes the compilation stop after phase " + phase), _phase(phase), _after(after) { }

    virtual ~PhaseStop() { }

    virtual std::string runsRightAfter()    const { return _after ? _phase : ""; }
    virtual std::string runsRightBefore()   const { return _after ? "" : _phase; }

    virtual bool run(PhaseContext& pctx) {
        return false;
    }

private:

    std::string _phase;
    bool _after;
};

Phase::Phase(const std::string& name, const std::string& descr) : _name(name), _descr(descr) {

}

Phase::~Phase() {

}

std::string Phase::runsRightAfter() const {
    return "";
}

std::string Phase::runsRightBefore() const {
    return "";
}

std::vector<std::string> Phase::runsAfter() const {
    return {};
}

std::vector<std::string> Phase::runsBefore() const {
    return {};
}

const std::string& Phase::getName() const {
    return _name;
}

const std::string& Phase::getDescr() const {
    return _descr;
}

std::shared_ptr<Phase> Phase::PrettyPrint(std::ostream& stream) {
    return std::shared_ptr<Phase>(new PhasePrettyPrint(stream));
}

std::shared_ptr<Phase> Phase::StopRightBefore(const std::string& phase) {
    return std::shared_ptr<Phase>(new PhaseStop(false, phase));
}

std::shared_ptr<Phase> Phase::StopRightAfter(const std::string& phase) {
    return std::shared_ptr<Phase>(new PhaseStop(true, phase));
}

}
