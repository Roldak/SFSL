#include "AbstractTestLogger.h"
#include "FileSystemTestGenerator.h"

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
        std::cout << "\t\t- " << testName << resultFor(success);
        if (!note.empty()) {
            std::cout << " [" << note << "]";
        }
        std::cout << std::endl;
    }

    virtual void testSuiteEnd(size_t passedTests, size_t totalTests) override {
        std::cout << std::endl << "\t\tPassed: " << passedTests << "/" << totalTests << std::endl;
    }

    virtual void testRunnerEnd(size_t passedSuites, size_t totalSuites) override {
        std::cout << std::endl << "\tPassed Suites: " << passedSuites << "/" << totalSuites << std::endl;
    }

private:

    std::string resultFor(bool success) {
        return success ? "SUCCESS" : "FAILURE";
    }

};

int main() {
    CoutLogger logger;
    test::FileSystemTestGenerator gen("sfsl");
    return gen.findAndGenerate()->run(logger);
}
