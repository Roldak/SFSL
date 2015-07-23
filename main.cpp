#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <ctime>
#include <memory.h>
#include "unistd.h"
#include "Compiler/Frontend/Lexer/Lexer.h"
#include "Compiler/Frontend/Parser/Parser.h"
#include "Compiler/Frontend/AST/Visitors/ASTPrinter.h"
#include "Compiler/Frontend/Analyser/NameAnalysis.h"
#include "Compiler/Frontend/Analyser/KindChecking.h"
#include "Compiler/Frontend/Analyser/TypeChecking.h"
#include "Compiler/Frontend/AST/Symbols/SymbolResolver.h"
#include "Compiler/Backend/BytecodeGenerator.h"

#define DEFAULT_CHUNK_SIZE 2048

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

    auto ctx = common::CompilationContext::DefaultCompilationContext(DEFAULT_CHUNK_SIZE);
    std::istringstream input(source);

    src::IStreamSource src(src::InputSourceName::make(ctx, sourceFile), input);

    try {

        lex::Lexer lexer(ctx, src, 1024);

        ast::Parser parser(ctx, lexer);

        std::cout << "STARTING PARSER" << std::endl;
        std::cout << ctx.get()->memoryManager().getInfos() << std::endl << std::endl;

        ast::Program* prog = parser.parse();

        if (ctx.get()->reporter().getErrorCount() != 0) {
            return 1;
        }

        /*ast::ASTPrinter printer(ctx, std::cout);
        prog->onVisit(&printer);*/

        std::cout << "STARTING SCOPE GENERATION" << std::endl;
        std::cout << ctx.get()->memoryManager().getInfos() << std::endl << std::endl;

        ast::ScopeGeneration scopeGen(ctx);
        prog->onVisit(&scopeGen);

        std::cout << "STARTING SYMBOL ASSIGNATION" << std::endl;
        std::cout << ctx.get()->memoryManager().getInfos() << std::endl << std::endl;

        ast::SymbolAssignation symAssign(ctx);
        prog->onVisit(&symAssign);

        if (ctx.get()->reporter().getErrorCount() != 0) {
            return 1;
        }

        std::cout << "STARTING KINDCHECKING" << std::endl;
        std::cout << ctx.get()->memoryManager().getInfos() << std::endl << std::endl;

        ast::KindChecking kindCheck(ctx);
        prog->onVisit(&kindCheck);

        if (ctx.get()->reporter().getErrorCount() != 0) {
            return 1;
        }

        sym::SymbolResolver res(prog, ctx);
        res.setPredefClassesPath("sfsl.lang");

        std::cout << "STARTING TYPECHECKING" << std::endl;
        std::cout << ctx.get()->memoryManager().getInfos() << std::endl << std::endl;

        ast::TypeChecking typeCheck(ctx, res);
        prog->onVisit(&typeCheck);

        if (ctx.get()->reporter().getErrorCount() != 0) {
            return 1;
        }

        std::cout << "STARTING BYTECODE GENERATION" << std::endl;
        std::cout << ctx.get()->memoryManager().getInfos() << std::endl << std::endl;

        out::LinkedListOutput<bc::BCInstruction*> out(ctx);
        bc::BytecodeGenerator gen(ctx, out);

        prog->onVisit(&gen);

        std::cout << "DONE" << std::endl;
        std::cout << ctx.get()->memoryManager().getInfos() << std::endl << std::endl;

        for (const bc::BCInstruction* i : out.toVector()) {
            std::cout << *i << std::endl;
        }

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
