//
//  Parser.cpp
//  SFSL
//
//  Created by Romain Beguet on 16.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Parser.h"

#include "../Lexer/Tokens/Litterals.h"

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

Identifier* Parser::parseIdentifier(const std::string& errMsg) {
    std::string name;

    if (isType(tok::TOK_ID)) {
        name = as<tok::Identifier>()->toString();
        accept();
    } else {
        expect<tok::TOK_TYPE>(tok::TOK_ID, "identifier");
    }

    return _mngr.New<Identifier>(name);
}

// Parsing

ASTNode* Parser::parseProgram() {
    std::vector<ModuleDecl*> modules;

    while (_lex.hasNext()) {
        expect<tok::KW_TYPE>(tok::KW_MODULE, "`module`", true);
        modules.push_back(parseModule());
    }

    return new Program(modules);
}

ModuleDecl* Parser::parseModule() {

    Identifier* moduleName = parseIdentifier("expected module name");
    std::vector<ASTNode*> decls;

    expect<tok::OPER_TYPE>(tok::OPER_L_BRACE, "`{`");

    if (accept(tok::KW_DEF)) {
        decls.push_back(parseDef());
    }

    expect<tok::OPER_TYPE>(tok::OPER_R_BRACE, "`}`");

    return _mngr.New<ModuleDecl>(moduleName, decls);
}

DefineDecl *Parser::parseDef() {

    Identifier* defName = parseIdentifier("expected definition name");

    expect<tok::OPER_TYPE>(tok::OPER_EQ, "`=`");

    Expression* expr = parseExpression();

    return _mngr.New<DefineDecl>(defName, expr);

}

Expression* Parser::parseExpression() {
    return parsePrimary();
}

Expression *Parser::parsePrimary() {
    Expression* toRet;

    switch (_currentToken->getTokenType()) {
    case tok::TOK_INT_LIT:
        toRet = new IntLitteral(as<tok::IntLitteral>()->getValue());
        accept();

        break;
    case tok::TOK_REAL_LIT:
        toRet = new RealLitteral(as<tok::RealLitteral>()->getValue());
        accept();
        break;

    case tok::TOK_STR_LIT:

    case tok::TOK_ID:
        toRet = parseIdentifier();
        break;

    case tok::TOK_KW:

    default:
        _ctx->reporter().error(*_currentToken,
                               "expected int litteral | real litteral | string litteral "
                               "| identifier | keyword; got " + _currentToken->toString());
    }

    return toRet;
}



}
