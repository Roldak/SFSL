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
#include <memory>
#include "../AST/Nodes/ProgramNode.h"
#include "../Lexer/Lexer.h"
#include "../Lexer/Tokens/Keyword.h"
#include "../Lexer/Tokens/Operators.h"
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
    Parser(std::shared_ptr<common::CompilationContext>& ctx, lex::Lexer& lexer);

    /**
     * @brief Start the parsing process
     */
    ast::ASTNode* parse();

private:

    // Utils

    bool isType(tok::TOK_TYPE type);

    bool accept(tok::OPER_TYPE type);
    bool accept(tok::KW_TYPE type);
    void accept();

    template<typename T>
    bool expect(T type, const std::string& expected, bool fatal = false);

    // Parsing

    ast::ASTNode* parseProgram();
    ast::ModuleNode* parseModule();

    // Members

    std::shared_ptr<common::CompilationContext> _ctx;
    common::AbstractMemoryManager& _mngr;
    lex::Lexer& _lex;

    tok::Token* _currentToken;

};

template<typename T>
bool Parser::expect(T type, const std::string& expected, bool fatal){
   tok::Token* lastPos = _currentToken;

   if (!accept(type)) {
       if (fatal) {
           _ctx->reporter().fatal(*lastPos, "expected '" + expected + "'; got '" + _currentToken->toString() + "'");
       } else {
           _ctx->reporter().error(*lastPos, "expected '" + expected + "'; got '" + _currentToken->toString() + "'");
       }
       return false;
   }
   return true;
}

}

#endif
