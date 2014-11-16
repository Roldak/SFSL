#include <iostream>
#include <sstream>
#include <vector>
#include <ctime>
#include <memory.h>
#include "Compiler/Lexer/Lexer.h"

using namespace std;
using namespace sfsl;

int main()
{
    std::string source = "module Main { def main() => { println(\"hello  world\" + 12.4); } }";

    clock_t exec = clock();

    std::shared_ptr<std::string> src_str(new std::string("hardcoded"));

    auto ctx = common::CompilationContext::DefaultCompilationContext();
    std::istringstream input(source);

    src::SFSLInputStream src(src_str.get(), input);

    lex::Lexer lexer(ctx, src);

    while (lexer.hasNext()) {
        cerr << lexer.getNext()->toStringDetailed() << endl;
    }

    ctx->reporter().info(*lexer.getNext(), "hello");

    cerr << "Execution Time : " << (clock() - exec)/(double)CLOCKS_PER_SEC << endl << endl;
}

