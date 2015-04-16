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
#include "Compiler/AST/Symbols/SymbolResolver.h"

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

    clock_t exec = clock();

    auto ctx = common::CompilationContext::DefaultCompilationContext();
    std::istringstream input(source);

    src::SFSLInputStream src(src::SFSLSourceName::make(ctx, sourceFile), input);

    try {

        lex::Lexer lexer(ctx, src, 1024);

        ast::Parser parser(ctx, lexer);

        ast::Program* prog = parser.parse();

        if (ctx.get()->reporter().getErrorCount() != 0) {
            return 1;
        }

        ast::ASTPrinter printer(ctx);
        prog->onVisit(&printer);

        ast::ScopeGeneration scopeGen(ctx);
        prog->onVisit(&scopeGen);

        ast::SymbolAssignation symAssign(ctx);
        prog->onVisit(&symAssign);

        sym::SymbolResolver res(prog, ctx);
        res.setPredefClassesPath("sfsl.lang");

        ast::TypeCheking typeCheck(ctx, res);
        prog->onVisit(&typeCheck);

        if (ctx.get()->reporter().getErrorCount() != 0) {
            return 1;
        }

    } catch(const sfsl::common::CompilationFatalError& ex) {
        std::cerr << ex.what() << std::endl;
    }

    if (compileOnly) {
        return 0;
    }

    std::cout << "Execution Time : " << (clock() - exec)/(double)CLOCKS_PER_SEC << std::endl << std::endl;
}
