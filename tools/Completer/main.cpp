#include <fstream>
#include "Compiler/Frontend/AST/Nodes/Program.h"
#include "Compiler/Frontend/AST/Visitors/ASTPrinter.h"
#include "Compiler/Frontend/Parser/Parser.h"
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

        complete::ScopeReporter id(ctx, complete::StandartScopeReporter::CoutReporter);
        prog->onVisit(&id);

        return true;
    }
};

class ScopeFinderPhase : public Phase {
public:
    ScopeFinderPhase(const std::string& toComplete)
        :   Phase("ScopeFinderPhase", "Only keeps scope that are parents of the given scope"),
            _toComplete(toComplete), _exprToComplete(nullptr) {
    }

    virtual ~ScopeFinderPhase() { }

    virtual std::vector<std::string> runsBefore() const {
        return {"NameAnalysis"};
    }

    virtual bool run(PhaseContext& pctx) {
        ast::Program* prog = pctx.require<ast::Program>("prog");
        CompCtx_Ptr ctx = *pctx.require<CompCtx_Ptr>("ctx");

        complete::ScopeFinder is(ctx, {1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0});
        prog->onVisit(&is);

        ast::Block* blk = is.getBlock();

        if (!blk) {
            return false;
        }

        src::StringSource source(src::InputSourceName::make(ctx, "tmp"), _toComplete);
        lex::Lexer toCompleteLexer(ctx, source);
        ast::Parser toCompleteParser(ctx, toCompleteLexer);
        ast::ASTPrinter printer(ctx, std::cout);

        _exprToComplete = toCompleteParser.parseSingleExpression();

        std::vector<ast::Expression*> exprs(blk->getStatements());
        exprs.push_back(_exprToComplete);

        *blk = ast::Block(exprs);

        blk->onVisit(&printer);

        return true;
    }

    ast::Expression* expressionToComplete() {
        return _exprToComplete;
    }

private:

    std::string _toComplete;
    ast::Expression* _exprToComplete;
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
        sourceFile = (char*)"Examples\\completion.sfsl";

    std::string source;

    std::ifstream f(sourceFile);
    while (f.good()) {
        source += f.get();
    }

    Compiler cmp(CompilerConfig(StandartReporter::EmptyReporter));
    EmptyCollector col;

    ProgramBuilder prog = cmp.parse(sourceFile, source);

    Module slang = prog.openModule("sfsl").openModule("lang");
    slang.typeDef("unit", cmp.classBuilder("unit").build());
    slang.typeDef("bool", cmp.classBuilder("bool").build());
    slang.typeDef("int", cmp.classBuilder("int").build());
    slang.typeDef("real", cmp.classBuilder("real").build());
    slang.typeDef("string", cmp.classBuilder("string").build());

    if (!complete && prog && false) {
        Pipeline ppl = Pipeline::createEmpty().insert(std::shared_ptr<Phase>(new ScopeIdentiferPhase));
        cmp.compile(prog, col, ppl);
    } else {
        std::shared_ptr<ScopeFinderPhase> completerPhase(std::shared_ptr<ScopeFinderPhase>(new ScopeFinderPhase("x")));
        Pipeline ppl = Pipeline::createDefault().insert(completerPhase);
        cmp.compile(prog, col, ppl);
        std::cout << completerPhase->expressionToComplete()->type()->toString() << std::endl;
    }

    return 0;
}
