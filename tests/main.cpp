#include "AbstractTestLogger.h"
#include "FileSystemTestGenerator.h"
#include "PhaseGraphTests.h"
#include "CanSubtypeTests.h"
#include "sfsl.h"

using namespace sfsl;

class CoutLogger : public test::AbstractTestLogger {
public:

    virtual ~CoutLogger() {

    }

    virtual void testRunnerStart(const std::string& testRunnerStart) override {
        _passed = 0;
        _total = 0;
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

    size_t _passed;
    size_t _total;
};

int main() {
    CoutLogger logger;
    test::buildPhaseGraphTests()->run(logger);
    test::buildCanSubtypeTests()->run(logger);
    test::FileSystemTestGenerator("sfsl").findAndGenerate()->run(logger);
    return 0;
}
