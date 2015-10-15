#include "AbstractTestLogger.h"
#include "FileSystemTestGenerator.h"
#include "sfsl.h"
#include "../src/Interface/PhaseGraph.h"

using namespace sfsl;

class CoutLogger : public test::AbstractTestLogger {
public:

    virtual ~CoutLogger() {

    }

    virtual void testRunnerStart(const std::string& testRunnerStart) override {
        std::cout << testRunnerStart << ":" << std::endl;
    }

    virtual void testSuiteStart(const std::string& testSuiteName) override {
        std::cout << "\t" << testSuiteName << ":" << std::endl;
    }

    virtual void result(const std::string& testName, bool success, const std::string& note) override {
        std::cout << "\t\t[" << resultFor(success) << "] " << testName;
        if (!note.empty()) {
            std::cout << " (" << note << ")";
        }
        std::cout << std::endl;

        if (success) {
            ++_passed;
        }
    }

    virtual void testSuiteEnd(size_t passedTests, size_t totalTests) override {
        std::cout << std::endl << "\t\tPassed: " << passedTests << "/" << totalTests << std::endl;
        _total += totalTests;
    }

    virtual void testRunnerEnd(size_t passedSuites, size_t totalSuites) override {
        std::cout << std::endl << "\tPassed Suites: " << passedSuites << "/" << totalSuites;
        std::cout << std::endl << "\tPassed Tests:  " << _passed << "/" << _total << std::endl;
    }

private:

    std::string resultFor(bool success) {
        return success ? "V" : "X";
    }

    size_t _passed {0};
    size_t _total {0};

};

class PhaseTest : public sfsl::Phase {
public:

    PhaseTest(const std::string& name,
              const std::string& rightAfter, const std::string& rightBefore,
              const std::vector<std::string>& afters = {}, const std::vector<std::string>& befores = {})
        : Phase(name, ""),
          _rightAfter(rightAfter), _rightBefore(rightBefore),
          _afters(afters), _befores(befores)
    { }

    virtual ~PhaseTest() {

    }

    virtual const std::string runsRightAfter() const {
        return _rightAfter;
    }

    virtual const std::string runsRightBefore() const {
        return _rightBefore;
    }

    virtual const std::vector<std::string> runsAfter() const {
        return _afters;
    }

    virtual const std::vector<std::string> runsBefore() const {
        return _befores;
    }

    virtual void run(PhaseContext& ctx) {
        std::cout << "hello from " << getName() << std::endl;
    }

private:

    const std::string _rightAfter;
    const std::string _rightBefore;

    const std::vector<std::string> _afters;
    const std::vector<std::string> _befores;
};

int main() {

    std::set<std::shared_ptr<Phase>> phases;

    phases.insert(std::shared_ptr<Phase>(new PhaseTest("Phase1", "", "")));
    phases.insert(std::shared_ptr<Phase>(new PhaseTest("Phase3", "Phase1", "")));
    phases.insert(std::shared_ptr<Phase>(new PhaseTest("Phase2", "Phase3", "")));
    phases.insert(std::shared_ptr<Phase>(new PhaseTest("Phase4", "Phase2", "")));
    phases.insert(std::shared_ptr<Phase>(new PhaseTest("Phase5", "", "", {}, {"Phase6"})));
    phases.insert(std::shared_ptr<Phase>(new PhaseTest("Phase6", "", "")));

    std::vector<std::shared_ptr<Phase>> sortedPhases(sortPhases(phases));

    for (std::shared_ptr<Phase> phase : sortedPhases) {
        std::cout << phase->getName() << " -> ";
    }

    std::cout << "END" << std::endl;

    return 0;

    CoutLogger logger;
    test::FileSystemTestGenerator gen("sfsl");
    return gen.findAndGenerate()->run(logger);
}
