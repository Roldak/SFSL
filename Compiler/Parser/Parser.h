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

#include "../Lexer/Lexer.h"
#include "../Lexer/Tokens/Keyword.h"
#include "../Lexer/Tokens/Operators.h"
#include "../Lexer/Tokens/Identifier.h"
#include "../Common/CompilationContext.h"

#include "../AST/Nodes/Program.h"
#include "../AST/Nodes/Statements.h"

namespace sfsl {

namespace ast {

/**
 * @brief The Parser is used to transform a sequence of tokens into an Abstract Syntax Tree
 */
class Parser {
public:

    /**
     * @brief Creates a Parser object
     * @param ctx The compilation context used throughout the parsing to report errors and allocate memory
     * @param lexer The lexer from which to fetch the tokens during the parsing
     */
    Parser(std::shared_ptr<common::CompilationContext>& ctx, lex::Lexer& lexer);

    /**
     * @brief Start the parsing process
     */
    ast::Program* parse();

private:

    // Utils

    bool isType(tok::TOK_TYPE type);

    bool accept(tok::TOK_TYPE type);
    bool accept(tok::OPER_TYPE type);
    bool accept(tok::KW_TYPE type);
    void accept();

    template<typename T>
    bool expect(T type, const std::string& expected, bool fatal = false);

    template<typename T>
    T* as();

    // Parsing

    ast::Identifier* parseIdentifier(const std::string& errMsg = "");

    ast::Program* parseProgram();
    ast::ModuleDecl* parseModule();
    ast::DefineDecl* parseDef(bool asStatement);

        // statements

    ast::Statement* parseStatement();

        // expressions

    ast::Expression* parseExpression();
    ast::Expression* parseBinary(ast::Expression* left, int precedence);
    ast::Expression* parsePrimary();

    ast::Block* parseBlock();
    ast::IfExpression* parseIf(bool asStatement);

    ast::Expression* parseSpecialBinaryContinuity(ast::Expression* left);
    ast::Tuple* parseTuple();
    ast::Tuple* parseTuple(std::vector<ast::Expression*>& exprs);
    ast::Expression* parseDotOperation(ast::Expression* left);

    // Members

    std::shared_ptr<common::CompilationContext> _ctx;
    common::AbstractMemoryManager& _mngr;
    lex::Lexer& _lex;

    size_t _lastTokenEndPos;
    tok::Token* _currentToken;

};

// Template methods implementations

template<typename T>
bool Parser::expect(T type, const std::string& expected, bool fatal) {
   tok::Token* lastTok = _currentToken;

   if (!accept(type)) {
       if (fatal) {
           _ctx->reporter().fatal(*lastTok, "expected " + expected + "; got `" + lastTok->toString() + "`");
       } else {
           _ctx->reporter().error(*lastTok, "expected " + expected + "; got `" + lastTok->toString() + "`");
       }
       return false;
   }

   return true;
}

template<typename T>
T* Parser::as() {
    return static_cast<T*>(_currentToken);
}

}

}

#endif
