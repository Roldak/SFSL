#include <iostream>
#include <fstream>
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
    std::string source;

    std::ifstream f("Examples\\test.sfsl");
    while (f.good())
        source += f.get();



    std::shared_ptr<std::string> src_str(new std::string("Examples\\test.sfsl"));

    while (true) {
clock_t exec = clock();
        auto ctx = common::CompilationContext::DefaultCompilationContext();
        std::istringstream input(source);

        src::SFSLInputStream src(src_str.get(), input);

        lex::Lexer lexer(ctx, src);

        while (lexer.hasNext()) {
            lexer.getNext();
        }

        //Parser parser(ctx, lexer);

        //ast::ASTNode* node = parser.parse();

        cout << "Execution Time : " << (clock() - exec)/(double)CLOCKS_PER_SEC << endl << endl;
    }
/*    std::cout << ((ast::Program*)node)->getModules().size() << std::endl;

    ast::ASTPrinter printer(ctx);

    node->onVisit(&printer);

    cout << "Execution Time : " << (clock() - exec)/(double)CLOCKS_PER_SEC << endl << endl;*/
}
