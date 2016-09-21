//
//  PhaseGraphTests.cpp
//  SFSL
//
//  Created by Romain Beguet on 13.07.16.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include <fstream>
#include <algorithm>

#include "sfsl.h"
#include "PhaseGraphTests.h"
#include "AbstractTest.h"
#include "../src/Interface/PhaseGraph.h"

namespace sfsl {

namespace test {

class GenericTestPhase : public Phase {
public:
    GenericTestPhase(const std::string& name) : Phase(name, "Test phase") { }

    virtual ~GenericTestPhase() {}

    virtual std::string runsRightAfter() const { return _runsRightAfter; }
    virtual std::string runsRightBefore() const { return _runsRightBefore; }
    virtual std::vector<std::string> runsAfter() const { return _runsAfter; }
    virtual std::vector<std::string> runsBefore() const { return _runsBefore; }

    virtual bool run(PhaseContext& pctx) { return true; }

    void setRunsRightAfter(const std::string& runsRightAfter) {
        _runsRightAfter = runsRightAfter;
    }

    void setRunsRightBefore(const std::string& runsRightBefore) {
        _runsRightBefore = runsRightBefore;
    }

    void addRunsAfter(const std::string& runsAfter) {
        _runsAfter.push_back(runsAfter);
    }

    void addRunsBefore(const std::string& runsBefore) {
        _runsBefore.push_back(runsBefore);
    }

private:

    std::string _runsRightAfter, _runsRightBefore;
    std::vector<std::string> _runsAfter, _runsBefore;
};

class PhaseGraphTest : public AbstractTest {
public:
    PhaseGraphTest(const std::string& name, bool shouldPass, const std::vector<std::string>& testDescr)
        : AbstractTest(name), _shouldPass(shouldPass) {
        buildTestFromTestDescription(testDescr);
    }

    ~PhaseGraphTest() { }

    bool run(AbstractTestLogger& logger) override {
        std::string message;
        bool success = !(checkExecutionOrder(message) ^ _shouldPass);
        logger.result(_name, success, message);
        return success;
    }

private:

    enum LINK_TYPE { RIGHT_AFTER, RIGHT_BEFORE, AFTER, BEFORE, UNKNOWN };

    static bool isSeparator(char c) {
        return c == ' ' || c == '-' || c == '<' || c == '>';
    }

    std::string parsePhaseName(std::string descr, size_t& cursor) {
        std::string phaseName;
        while (cursor < descr.size() && !isSeparator(descr[cursor])) {
            phaseName += descr[cursor];
            ++cursor;
        }
        return phaseName;
    }

    void eatBlanks(std::string descr, size_t& cursor) {
        while (cursor < descr.size() && descr[cursor] == ' ') {
            ++cursor;
        }
    }

    LINK_TYPE parseLinkType(std::string descr, size_t& cursor) {
        if (descr[cursor] == '-') {
            ++cursor;
            if (descr[cursor] == '>') {
                ++cursor;
                return RIGHT_BEFORE;
            } else if (descr[cursor] == '-') {
                ++cursor;
                if (descr[cursor] == '>') {
                    ++cursor;
                    return BEFORE;
                }
            }
        } else if (descr[cursor] == '<') {
            ++cursor;
            if (descr[cursor] == '-') {
                ++cursor;
                if (descr[cursor] == '-') {
                    ++cursor;
                    return AFTER;
                } else {
                    return RIGHT_AFTER;
                }
            }
        }

        return UNKNOWN;
    }

    GenericTestPhase* getOrCreate(std::map<std::string, std::shared_ptr<GenericTestPhase>>& phases, const std::string& phase) {
        auto it = phases.find(phase);
        if (it != phases.end()) {
            return it->second.get();
        } else {
            GenericTestPhase* genPhase = new GenericTestPhase(phase);
            phases.insert(std::make_pair(phase, std::shared_ptr<GenericTestPhase>(genPhase)));
            return genPhase;
        }
    }

    void buildTestFromTestDescription(const std::vector<std::string>& testDescr) {
        std::map<std::string, std::shared_ptr<GenericTestPhase>> phases;

        for (std::string descr : testDescr) {
            size_t cursor = 0;
            GenericTestPhase* sourcePhase = getOrCreate(phases, parsePhaseName(descr, cursor));
            eatBlanks(descr, cursor);

            do {
                LINK_TYPE ltype = parseLinkType(descr, cursor);
                eatBlanks(descr, cursor);

                GenericTestPhase* destPhase = getOrCreate(phases, parsePhaseName(descr, cursor));
                eatBlanks(descr, cursor);

                switch (ltype) {
                case RIGHT_AFTER:   sourcePhase->setRunsRightAfter(destPhase->getName()); break;
                case RIGHT_BEFORE:  sourcePhase->setRunsRightBefore(destPhase->getName()); break;
                case AFTER:         sourcePhase->addRunsAfter(destPhase->getName()); break;
                case BEFORE:        sourcePhase->addRunsBefore(destPhase->getName()); break;
                default:            break;
                }

                sourcePhase = destPhase;
            } while (cursor < descr.size());
        }

        for (const auto& phase : phases) {
            _phases.insert(phase.second);
        }
    }

    bool checkExecutionOrder(std::string& message) const {
        try {
            std::vector<std::shared_ptr<Phase>> sortedPhases(sortPhases(_phases));

            if (sortedPhases.size() != _phases.size()) {
                message = "Expected " + std::to_string(_phases.size()) + " phases, got " + std::to_string(sortedPhases.size()) + ".";
                return false;
            }

            for (size_t i = 0; i < sortedPhases.size(); ++i) {
                std::shared_ptr<Phase> phase = sortedPhases[i];

                if (!phase->runsRightAfter().empty() && !isPhaseAt(sortedPhases, i - 1, phase->runsRightAfter())) {
                    message = "Expected phase `" + phase->runsRightAfter() + "` to run right before `" + phase->getName() + "`.";
                    return false;
                }

                if (!phase->runsRightBefore().empty() && !isPhaseAt(sortedPhases, i + 1, phase->runsRightBefore())) {
                    message = "Expected phase `" + phase->runsRightAfter() + "` to run right after `" + phase->getName() + "`.";
                    return false;
                }

                for (const std::string& pbefore : phase->runsAfter()) {
                    if (!containsBefore(sortedPhases, i, pbefore)) {
                        message = "Expected phase `" + pbefore + "` to run before `" + phase->getName() + "`.";
                        return false;
                    }
                }

                for (const std::string& pafter : phase->runsBefore()) {
                    if (!containsAfter(sortedPhases, i, pafter)) {
                        message = "Expected phase `" + pafter + "` to run after `" + phase->getName() + "`.";
                        return false;
                    }
                }
            }

            message = "{";
            for (std::shared_ptr<Phase> phase : sortedPhases) {
                message += phase->getName() + " -> ";
            }
            message += "END}";
            return true;
        } catch (PhaseGraphResolutionError& err) {
            message = err.what();
            return false;
        }
    }

    static bool isPhaseAt(const std::vector<std::shared_ptr<Phase>>& phases, size_t i, const std::string& phaseName) {
        if (i < 0 || i >= phases.size()) {
            return false;
        } else {
            return phases[i]->getName() == phaseName;
        }
    }

    static bool containsAfter(const std::vector<std::shared_ptr<Phase>>& phases, size_t i, const std::string& phaseName) {
        ++i;
        for (; i < phases.size(); ++i) {
            if (isPhaseAt(phases, i, phaseName)) {
                return true;
            }
        }
        return false;
    }

    static bool containsBefore(const std::vector<std::shared_ptr<Phase>>& phases, size_t i, const std::string& phaseName) {
        --i;
        for (; i < (size_t) -1; --i) {
            if (isPhaseAt(phases, i, phaseName)) {
                return true;
            }
        }
        return false;
    }

    bool _shouldPass;
    std::set<std::shared_ptr<Phase>> _phases;
};

TestRunner* buildPhaseGraphTests() {
    TestSuiteBuilder basic("Basic");

    basic.addTest(new PhaseGraphTest("Basic1", true, {"A-->B-->C"}));
    basic.addTest(new PhaseGraphTest("Basic2", true, {"C<--B<--A"}));
    basic.addTest(new PhaseGraphTest("Basic3", true, {"A->B<--C"}));
    basic.addTest(new PhaseGraphTest("Basic4", true, {"C-->B<--A"}));
    basic.addTest(new PhaseGraphTest("Concrete1", true, {"NameAnalysis-->KindChecking-->TypeChecking-->PreTransform-->AST2BAST-->CodeGen"}));
    basic.addTest(new PhaseGraphTest("Concrete2", true, {"CodeGen<--AST2BAST<--PreTransform<--TypeChecking<--KindChecking<--NameAnalysis"}));

    TestSuiteBuilder medium("Medium");

    medium.addTest(new PhaseGraphTest("Medium1", true, {"A-->C", "B-->C"}));
    medium.addTest(new PhaseGraphTest("Medium2", false, {"A->B", "B-->C", "A<--C"}));
    medium.addTest(new PhaseGraphTest("Medium3", true, {"A->B", "B-->C", "C<--A"}));

    TestSuiteBuilder advanced("Advanced");

    advanced.addTest(new PhaseGraphTest("Advanced1", true, {"A-->B-->C-->D", "X-->E-->C", "A-->X", "X-->D"}));
    advanced.addTest(new PhaseGraphTest("Concrete1", true, {
                                            "CodeGen<--AST2BAST<--PreTransform<--TypeChecking<--KindChecking<--NameAnalysis",
                                            "UsageAnalysis<--TypeChecking",
                                            "UsageAnalysis-->PreTransform"}));

    advanced.addTest(new PhaseGraphTest("Concrete2", true, {
                                            "CodeGen<--AST2BAST<--PreTransform<--TypeChecking<--KindChecking<--NameAnalysis",
                                            "StopRightBeforeCodeGen->CodeGen",
                                            "DivByZero<-NameAnalysis",
                                            "UsageAnalysis<--TypeChecking",
                                            "UsageAnalysis-->PreTransform"}));

    return new TestRunner("PhaseGraphTests", {basic.build(), medium.build(), advanced.build()});
}

}

}
