#include <iostream>
#include <sstream>
#include <vector>
#include <ctime>
#include <memory.h>
#include "Compiler/Lexer/Lexer.h"
#include "Compiler/Parser/Parser.h"
#include "Compiler/AST/Visitors/ASTPrinter.h"

using namespace std;
using namespace sfsl;

int main()
{
    std::string source = "module Main { def main(args) => { println(args); if (f(args)) println(1) else println(2); println(if (true) 1 else 2); } }";

    clock_t exec = clock();

    std::shared_ptr<std::string> src_str(new std::string("hardcoded"));

    auto ctx = common::CompilationContext::DefaultCompilationContext();
    std::istringstream input(source);

    src::SFSLInputStream src(src_str.get(), input);

    lex::Lexer lexer(ctx, src);
    Parser parser(ctx, lexer);

    ast::ASTNode* node = parser.parse();

    ast::ASTPrinter printer(ctx);

    node->onVisit(&printer);

    cerr << "Execution Time : " << (clock() - exec)/(double)CLOCKS_PER_SEC << endl << endl;
}

