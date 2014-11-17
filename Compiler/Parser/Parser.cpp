//
//  Parser.cpp
//  SFSL
//
//  Created by Romain Beguet on 16.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Parser.h"

namespace sfsl {

using namespace tok;
using namespace ast;

Parser::Parser(std::shared_ptr<common::CompilationContext>& ctx, lex::Lexer &lexer)
    : _ctx(ctx), _mngr(ctx->memoryManager()), _lex(lexer), _currentToken(nullptr) {

}

ASTNode* Parser::parse() {
    _currentToken = _lex.getNext();
    return parseProgram();
}

// Utils

bool Parser::isType(TOK_TYPE type) {
    return _currentToken->getTokenType() == type;
}

bool Parser::accept(OPER_TYPE type) {
    bool toRet = isType(TOK_OPER) && ((Operator*)_currentToken)->getOpType() == type;
    accept();
    return toRet;
}

bool Parser::accept(KW_TYPE type) {
    bool toRet = isType(TOK_KW) && ((Keyword*)_currentToken)->getKwType() == type;
    accept();
    return toRet;
}

void Parser::accept() {
    _currentToken = _lex.getNext();
}

// Parsing

ASTNode* Parser::parseProgram() {
    std::vector<ModuleNode*> modules;

    while (_lex.hasNext()) {
        expect<KW_TYPE>(KW_MODULE, "module", true);
        modules.push_back(parseModule());
    }

    return new ProgramNode(modules);
}

ModuleNode* Parser::parseModule() {
    expect<OPER_TYPE>(OPER_L_BRACE, "{");



    expect<OPER_TYPE>(OPER_R_BRACE, "}");

    return _mngr.New<ModuleNode>();
}



}
