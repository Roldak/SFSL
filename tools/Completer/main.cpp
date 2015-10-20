#include <fstream>
#include "Compiler/Frontend/AST/Nodes/Program.h"
#include "Visitors/ScopeIdentifier.h"
#include "ScopeReporter.h"
#include "sfsl.h"
#include "unistd.h"

using namespace sfsl;

class ScopeIdentiferPhase : public Phase {
public:
    ScopeIdentiferPhase() : Phase("ScopeIdentifier", "Assigns scopes unique ids") { }
    virtual ~ScopeIdentiferPhase() { }

    virtual bool run(PhaseContext& pctx) {
        ast::Program* prog = pctx.require<ast::Program>("prog");
        CompCtx_Ptr ctx = *pctx.require<CompCtx_Ptr>("ctx");

        complete::ScopeIdentifer id(ctx, complete::StandartScopeReporter::CoutReporter);
        prog->onVisit(&id);

        return true;
    }
};

int main(int argc, char** argv) {
    // LOAD FILE

    char* sourceFile = NULL;
    char* outputFile = NULL;
    bool complete = false;
    int option;

    while((option = getopt(argc, argv, "s:c:g")) != -1){
        switch (option) {
        case 's':
            sourceFile = optarg;
            break;
        case 'c':
            complete = true;
            break;
        case 'g':
            outputFile = optarg;
            break;
        default:
            std::cerr << "unexpected program argument : " << option << std::endl;
        }
    }

    if (!complete && !outputFile && false) {
        std::cerr << "missing option -c or -g filename" << std::endl;
        return 1;
    }

    if (!sourceFile)
        sourceFile = (char*)"Examples\\test.sfsl";

    std::string source;

    std::ifstream f(sourceFile);
    while (f.good()) {
        source += f.get();
    }

    Compiler cmp(CompilerConfig(StandartReporter::EmptyReporter));
    Pipeline ppl = Pipeline::createEmpty();
    EmptyCollector col;

    ProgramBuilder prog = cmp.parse(sourceFile, source);

    Module slang = prog.openModule("sfsl").openModule("lang");
    slang.typeDef("unit", cmp.classBuilder("unit").build());
    slang.typeDef("bool", cmp.classBuilder("bool").build());
    slang.typeDef("int", cmp.classBuilder("int").build());
    slang.typeDef("real", cmp.classBuilder("real").build());
    slang.typeDef("string", cmp.classBuilder("string").build());

    if (!complete && prog) {
        ppl.insert(std::shared_ptr<Phase>(new ScopeIdentiferPhase));
        cmp.compile(prog, col, ppl);
    } else {

    }

    return 0;
}
