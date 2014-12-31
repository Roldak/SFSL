#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <ctime>
#include <memory.h>
#include "unistd.h"
#include "Compiler/Lexer/Lexer.h"
#include "Compiler/Parser/Parser.h"
#include "Compiler/AST/Visitors/ASTPrinter.h"
#include "Compiler/Analyser/NameAnalysis.h"
#include "Compiler/Analyser/TypeChecking.h"

using namespace std;
using namespace sfsl;

int main(int argc, char** argv) {
    // LOAD FILE

    char* sourceFile = NULL;
    bool compileOnly = false;
    int option;

    while((option = getopt(argc, argv, "s:c")) != -1){
        switch (option) {
        case 's':
            sourceFile = optarg;
            break;
        case 'c':
            compileOnly = true;
            break;
        default:
            std::cerr<<"unexpected program argument : " << option << std::endl;
        }
    }

    if (!sourceFile)
        sourceFile = (char*)"Examples\\test.sfsl";

    std::string source;

    std::ifstream f(sourceFile);
    while (f.good()) {
        source += f.get();
    }

    std::shared_ptr<std::string> src_str(new std::string(sourceFile));

    clock_t exec = clock();

    auto ctx = common::CompilationContext::DefaultCompilationContext();
    std::istringstream input(source);

    src::SFSLInputStream src(src_str.get(), input);

    lex::Lexer lexer(ctx, src, 1024);

    ast::Parser parser(ctx, lexer);

    ast::Program* prog = parser.parse();

    if (ctx.get()->reporter().getErrorCount() == 0) {
        ast::ASTPrinter printer(ctx);
        prog->onVisit(&printer);

        ast::ScopeGeneration scopeGen(ctx);
        prog->onVisit(&scopeGen);

        ast::SymbolAssignation symAssign(ctx);
        prog->onVisit(&symAssign);

        ast::TypeAssignation typeAssign(ctx);
        prog->onVisit(&typeAssign);
    }

    if (compileOnly) {
        return 0;
    }

    cout << "Execution Time : " << (clock() - exec)/(double)CLOCKS_PER_SEC << endl << endl;
}
