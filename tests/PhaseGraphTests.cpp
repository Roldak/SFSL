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

namespace sfsl {

namespace test {

class ExecutionTracker {
public:
    ExecutionTracker(){

    }

    void append(const Phase* phase) {
        _phases.push_back(phase);
    }

    size_t phaseCount() const {
        return _phases.size();
    }

    Phase* getPhaseAt(size_t i) const {
        return _phases[i];
    }

    bool isPhaseAt(size_t i, const std::string& phaseName) const {
        if (i < 0 || i >= _phases.size()) {
            return false;
        } else {
            return _phases[i]->getName() == phaseName;
        }
    }

    bool containsAfter(size_t i, const std::string& phaseName) const {
        ++i;
        for (; i < _phases.size(); ++i) {
            if (isPhaseAt(i, phaseName)) {
                return true;
            }
        }
        return false;
    }

    bool containsBefore(size_t i, const std::string& phaseName) const {
        --i;
        for (; i < (size_t) -1; --i) {
            if (isPhaseAt(i, phaseName)) {
                return true;
            }
        }
        return false;
    }

    std::vector<Phase*> _phases;
};

class GenericTestPhase : Phase {
public:
    GenericTestPhase(
            const std::string& name,
            const std::string& runsRightAfter = "", const std::string& runsRightBefore = "",
            const std::vector<std::string>& runsAfter = {}, const std::vector<std::string>& runsBefore = {})
        : Phase(name, ""),
          _runsRightAfter(runsRightAfter), _runsRightBefore(runsRightBefore),
          _runsAfter(runsAfter), _runsBefore(runsBefore)
    {

    }

    virtual ~GenericTestPhase() {}

    virtual std::string runsRightAfter() const { return _runsRightAfter; }
    virtual std::string runsRightBefore() const { return _runsRightBefore; }
    virtual std::vector<std::string> runsAfter() const { return _runsAfter; }
    virtual std::vector<std::string> runsBefore() const { return _runsBefore; }

    virtual bool run(PhaseContext& pctx) {
        ExecutionTracker* execTracker = pctx.require<ExecutionTracker>("execTracker");
        execTracker->append(this);
        return true;
    }

private:

    std::string _runsRightAfter, _runsRightBefore;
    std::vector<std::string> _runsAfter, _runsBefore;
};

bool checkExecutionOrder(const ExecutionTracker& tracker, std::string& error) {
    for (size_t i = 0; i < tracker.phaseCount(); ++i) {
        Phase phase = tracker.getPhaseAt(i);

        if (!phase.runsRightAfter().empty() && !tracker.isPhaseAt(i - 1, phase.runsRightAfter())) {
            error = "Expected phase `" + phase.runsRightAfter() + "` to run right before `" + phase.getName() + "`.";
            return false;
        }

        if (!phase.runsRightBefore().empty() && !tracker.isPhaseAt(i + 1, phase.runsRightBefore())) {
            error = "Expected phase `" + phase.runsRightAfter() + "` to run right after `" + phase.getName() + "`.";
            return false;
        }

        for (const std::string& pbefore : phase.runsAfter()) {
            if (!tracker.containsBefore(i, pbefore)) {
                error = "Expected phase `" + pbefore + "` to run before `" + phase.getName() + "`.";
                return false;
            }
        }

        for (const std::string& pafter : phase.runsBefore()) {
            if (!tracker.containsAfter(i, pafter)) {
                error = "Expected phase `" + pafter + "` to run after `" + phase.getName() + "`.";
                return false;
            }
        }

        return true;
    }
}

class PhaseGraphTest : public AbstractTest {
public:
    PhaseGraphTest(const std::string& name, bool shouldPass)
        : AbstractTest(name), _shouldPass(shouldPass) {

    }

    virtual ~PhaseGraphTest();

    virtual bool run(AbstractTestLogger& logger) override {

    }

private:

    bool _shouldPass;
};

TestRunner* buildPhaseGraphTests() {
    TestSuiteBuilder builder("Basic");



    return new TestRunner("Phase Graph Tests", {builder.build()});
}

}

}
