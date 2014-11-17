//
//  Parser.cpp
//  SFSL
//
//  Created by Romain Beguet on 16.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Parser.h"

namespace sfsl {

using namespace ast;

Parser::Parser(std::shared_ptr<common::CompilationContext>& ctx, lex::Lexer &lexer)
    : _ctx(ctx), _mngr(ctx->memoryManager()), _lex(lexer), _currentToken(nullptr) {

}

ASTNode* Parser::parse() {
    _currentToken = _lex.getNext();
    return parseProgram();
}

// Utils

bool Parser::isType(tok::TOK_TYPE type) {
    return _currentToken->getTokenType() == type;
}

bool Parser::accept(tok::TOK_TYPE type) {
    bool toRet = isType(type);
    accept();
    return toRet;
}

bool Parser::accept(tok::OPER_TYPE type) {
    bool toRet = isType(tok::TOK_OPER) && ((tok::Operator*)_currentToken)->getOpType() == type;
    accept();
    return toRet;
}

bool Parser::accept(tok::KW_TYPE type) {
    bool toRet = isType(tok::TOK_KW) && ((tok::Keyword*)_currentToken)->getKwType() == type;
    accept();
    return toRet;
}

void Parser::accept() {
    _currentToken = _lex.getNext();
}

Identifier *Parser::parseIdentifier(const std::string &errMsg) {
    std::string name;

    if (isType(tok::TOK_ID)) {
        name = as<tok::Identifier>()->toString();
    } else {
        expect<tok::TOK_TYPE>(tok::TOK_ID, "Identifier");
    }

    return _mngr.New<Identifier>(name);
}

// Parsing

ASTNode* Parser::parseProgram() {
    std::vector<Module*> modules;

    while (_lex.hasNext()) {
        expect<tok::KW_TYPE>(tok::KW_MODULE, "'module'", true);
        modules.push_back(parseModule());
    }

    return new Program(modules);
}

Module *Parser::parseModule() {

    Identifier* moduleName = parseIdentifier("expected module name");

    expect<tok::OPER_TYPE>(tok::OPER_L_BRACE, "'{'");



    expect<tok::OPER_TYPE>(tok::OPER_R_BRACE, "'}'");

    return _mngr.New<Module>(moduleName);
}



}
