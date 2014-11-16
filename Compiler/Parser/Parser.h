//
//  Parser.h
//  SFSL
//
//  Created by Romain Beguet on 16.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Parser__
#define __SFSL__Parser__

#include <iostream>
#include "../AST/Nodes/ASTNode.h"
#include "../Lexer/Lexer.h"
#include "../Common/CompilationContext.h"

namespace sfsl {

/**
 * @brief The Parser is used to transform a sequence of tokens into an Abstract Syntax Tree
 */
class Parser {
public:

    /**
     * @brief Creates a Parser object
     * @param ctx The compilation context used throughout the parsing to report errors or allocate memory
     * @param lexer The lexer from which to fetch the tokens during the parsing
     */
    Parser(common::CompilationContext& ctx, lex::Lexer& lexer);

    /**
     * @brief Start the parsing process
     */
    void parse();

private:

    common::CompilationContext& _ctx;
    lex::Lexer& _lex;

};

}

#endif
