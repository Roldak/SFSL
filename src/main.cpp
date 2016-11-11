#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include "unistd.h"
#include "sfsl.h"

#define DEFAULT_CHUNK_SIZE 2048

using namespace sfsl;

int main(int argc, char** argv) {
    // LOAD FILE

    char* sourceFile = NULL;
    bool checkOnly = false;
    int option;

    while((option = getopt(argc, argv, "c")) != -1) {
        switch (option) {
        case 'c':
            checkOnly = true;
            break;
        default:
            std::cerr << "unexpected program argument : " << option << std::endl;
            break;
        }
    }

    if (optind < argc) {
        sourceFile = argv[optind++];
    } else {
        std::cerr << "missing source file" << std::endl;
        return 1;
    }

    std::ifstream f(sourceFile);
    std::stringstream buffer;
    buffer << f.rdbuf();

    std::string source = buffer.str();

    Compiler cmp(CompilerConfig()
                 .with<opt::Reporter>(StandartReporter::CerrReporter)
                 .with<opt::AfterEachPhase>(opt::AfterEachPhase::print(std::cout, opt::AfterEachPhase::ExecutionTime | opt::AfterEachPhase::MemoryInfos))
                 .with<opt::AtEnd>(opt::AtEnd::print(std::cout, opt::AtEnd::CompilationTime | opt::AtEnd::MemoryInfos)));

    Pipeline ppl = Pipeline::createDefault();

    ByteCodeCollector bcc;
    EmptyCollector emc;
    AbstractOutputCollector* col;

    try {
        cmp.loadPlugin(STDLIBNAME);

        ProgramBuilder builder = cmp.parse(sourceFile, source);

        if (checkOnly) {
            col = &emc;
            ppl.insert(Phase::StopRightBefore("PreTransform"));
        } else {
            col = &bcc;
        }

        cmp.compile(builder, *col, ppl);

        if (!checkOnly) {
            for (const std::string& i : bcc.get()) {
                std::cout << i << std::endl;
            }
        }

    } catch(const CompileError& ex) {
        std::cerr << ex.what() << std::endl;
    }
}
