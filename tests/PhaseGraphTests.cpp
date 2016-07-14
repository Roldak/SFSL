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
#include "Interface/PhaseGraph.h"

namespace sfsl {

namespace test {

class GenericTestPhase : Phase {
public:
    GenericTestPhase(
            const std::string& name,
            const std::string& runsRightAfter = "", const std::string& runsRightBefore = "",
            const std::vector<std::string>& runsAfter = {}, const std::vector<std::string>& runsBefore = {})
        : Phase(name, ""),
          _runsRightAfter(runsRightAfter), _runsRightBefore(runsRightBefore),
          _runsAfter(runsAfter), _runsBefore(runsBefore) { }

    virtual ~GenericTestPhase() {}

    virtual std::string runsRightAfter() const { return _runsRightAfter; }
    virtual std::string runsRightBefore() const { return _runsRightBefore; }
    virtual std::vector<std::string> runsAfter() const { return _runsAfter; }
    virtual std::vector<std::string> runsBefore() const { return _runsBefore; }

    virtual bool run(PhaseContext& pctx) { return true; }

private:

    std::string _runsRightAfter, _runsRightBefore;
    std::vector<std::string> _runsAfter, _runsBefore;
};

class PhaseGraphTest : public AbstractTest {
public:
    PhaseGraphTest(const std::string& name, bool shouldPass)
        : AbstractTest(name), _shouldPass(shouldPass) {

    }

    virtual ~PhaseGraphTest() { }

    virtual bool run(AbstractTestLogger& logger) override {
        std::string error;
        bool success = !(checkExecutionOrder(error) ^ _shouldPass);
        logger.result(_name, success, success ? "" : error);
        return success;
    }

private:

    bool checkExecutionOrder(std::string& error) const {
        try {
            std::vector<std::shared_ptr<Phase>> sortedPhases = sortPhases(_phases);

            for (size_t i = 0; i < sortedPhases.size(); ++i) {
                std::shared_ptr<Phase> phase = sortedPhases[i];

                if (!phase->runsRightAfter().empty() && !isPhaseAt(sortedPhases, i - 1, phase->runsRightAfter())) {
                    error = "Expected phase `" + phase->runsRightAfter() + "` to run right before `" + phase->getName() + "`.";
                    return false;
                }

                if (!phase->runsRightBefore().empty() && !isPhaseAt(sortedPhases, i + 1, phase->runsRightBefore())) {
                    error = "Expected phase `" + phase->runsRightAfter() + "` to run right after `" + phase->getName() + "`.";
                    return false;
                }

                for (const std::string& pbefore : phase->runsAfter()) {
                    if (!containsBefore(sortedPhases, i, pbefore)) {
                        error = "Expected phase `" + pbefore + "` to run before `" + phase->getName() + "`.";
                        return false;
                    }
                }

                for (const std::string& pafter : phase->runsBefore()) {
                    if (!containsAfter(sortedPhases, i, pafter)) {
                        error = "Expected phase `" + pafter + "` to run after `" + phase->getName() + "`.";
                        return false;
                    }
                }
            }
            return true;
        } catch (PhaseGraphResolutionError& err) {
            error = err.what();
            return false;
        }
    }

    static bool isPhaseAt(const std::vector<std::shared_ptr<Phase>>& phases, size_t i, const std::string& phaseName) const {
        if (i < 0 || i >= phases.size()) {
            return false;
        } else {
            return phases[i]->getName() == phaseName;
        }
    }

    static bool containsAfter(const std::vector<std::shared_ptr<Phase>>& phases, size_t i, const std::string& phaseName) const {
        ++i;
        for (; i < phases.size(); ++i) {
            if (isPhaseAt(phases, i, phaseName)) {
                return true;
            }
        }
        return false;
    }

    static bool containsBefore(const std::vector<std::shared_ptr<Phase>>& phases, size_t i, const std::string& phaseName) const {
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
    TestSuiteBuilder builder("Basic");



    return new TestRunner("Phase Graph Tests", {builder.build()});
}

}

}
