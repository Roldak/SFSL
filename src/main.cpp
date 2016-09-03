#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <ctime>
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

    while((option = getopt(argc, argv, "s:c")) != -1){
        switch (option) {
        case 's':
            sourceFile = optarg;
            break;
        case 'c':
            checkOnly = true;
            break;
        default:
            std::cerr << "unexpected program argument : " << option << std::endl;
        }
    }

    if (!sourceFile)
        sourceFile = (char*)"Examples\\test.sfsl";

    std::string source;

    std::ifstream f(sourceFile);
    while (f.good()) {
        source += f.get();
    }

    clock_t exec = clock();

    Compiler cmp(CompilerConfig()
                 .with<opt::Reporter>(StandartReporter::CerrReporter)
                 .with<opt::PrintCompilationTime>(opt::Frequency::AfterEachPhase)
                 .with<opt::PrintMemoryUsage>(opt::Frequency::AfterEachPhase));

    Pipeline ppl = Pipeline::createDefault();

    ByteCodeCollector bcc;
    EmptyCollector emc;
    AbstractOutputCollector* col;

    try {
        cmp.loadPlugin(STDLIBNAME);

        ProgramBuilder builder = cmp.parse(sourceFile, source);

        if (checkOnly) {
            col = &emc;
            ppl.insert(Phase::StopRightBefore("CodeGen"));
        } else {
            col = &bcc;
        }

        cmp.compile(builder, *col, ppl);

        if (!checkOnly) {
            for (const std::string& i : bcc.get()) {
                std::cout << i << std::endl;
            }

            std::cout << "Compilation Time : " << (clock() - exec)/(double)CLOCKS_PER_SEC << std::endl << std::endl;
        }

    } catch(const CompileError& ex) {
        std::cerr << ex.what() << std::endl;
    }
}
