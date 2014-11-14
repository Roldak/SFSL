#include <iostream>
#include <sstream>
#include <vector>

#include "Compiler/Lexer/Lexer.h"

using namespace std;
using namespace sfsl;

int main()
{
    std::string source = "module Main{ def main() => 3.141592653589793 { /* helloasdasd njndf \n asdasd*/ print $\\ ln(\"hello world\" + 12); // } }";

    common::MemoryManager<> mem_mngr;

    std::istringstream input(source);
    src::SFSLInputStream src("hardcoded", input);

    lex::Lexer lexer(mem_mngr, src);

    while (lexer.hasNext()) {
        std::cout << lexer.getNext()->toStringDetailed() << std::endl;
    }

}

