#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <ctime>
#include <memory.h>
#include "Compiler/Lexer/Lexer.h"
#include "Compiler/Parser/Parser.h"
#include "Compiler/AST/Visitors/ASTPrinter.h"
#include "Compiler/Analyser/NameAnalysis.h"

using namespace std;
using namespace sfsl;

int main()
{
    std::string source;

    std::ifstream f("Examples\\test.sfsl");
    while (f.good()) {
        source += f.get();
    }

    std::shared_ptr<std::string> src_str(new std::string("Examples\\test.sfsl"));

    clock_t exec = clock();

    auto ctx = common::CompilationContext::DefaultCompilationContext();
    std::istringstream input(source);

    src::SFSLInputStream src(src_str.get(), input);

    lex::Lexer lexer(ctx, src, 1024);
    ast::Parser parser(ctx, lexer);

    ast::ASTNode* node = parser.parse();

    ast::ASTPrinter printer(ctx);
    node->onVisit(&printer);

    ast::ScopeGeneration scopeGen(ctx);
    node->onVisit(&scopeGen);

    cout << ((ast::Program*)node)->getModules()[0]->getSymbol()->getDefinition("test") << endl;

    cout << "Execution Time : " << (clock() - exec)/(double)CLOCKS_PER_SEC << endl << endl;
}
