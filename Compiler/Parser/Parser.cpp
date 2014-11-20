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
    if (toRet) accept();
    return toRet;
}

bool Parser::accept(tok::OPER_TYPE type) {
    bool toRet = isType(tok::TOK_OPER) && ((tok::Operator*)_currentToken)->getOpType() == type;
    if (toRet) accept();
    return toRet;
}

bool Parser::accept(tok::KW_TYPE type) {
    bool toRet = isType(tok::TOK_KW) && ((tok::Keyword*)_currentToken)->getKwType() == type;
    if (toRet) accept();
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
        expect(tok::TOK_ID, "identifier");
    }

    return _mngr.New<Identifier>(name);
}

// Parsing

ASTNode* Parser::parseProgram() {
    std::vector<ModuleDecl*> modules;

    while (_lex.hasNext()) {
        expect(tok::KW_MODULE, "`module`", true);
        modules.push_back(parseModule());
    }

    return new Program(modules);
}

ModuleDecl* Parser::parseModule() {

    Identifier* moduleName = parseIdentifier("expected module name");
    std::vector<ASTNode*> decls;

    expect(tok::OPER_L_BRACE, "`{`");

    while (accept(tok::KW_DEF)) {
        decls.push_back(parseDef());
    }

    expect(tok::OPER_R_BRACE, "`}`");

    return _mngr.New<ModuleDecl>(moduleName, decls);
}

DefineDecl* Parser::parseDef() {

    Identifier* defName = parseIdentifier("expected definition name");

    if (!(isType(tok::TOK_OPER) && as<tok::Operator>()->getOpType() == tok::OPER_L_PAREN))
        expect(tok::OPER_EQ, "`=`");

    Expression* expr = parseExpression();

    return _mngr.New<DefineDecl>(defName, expr);

}

Statement* Parser::parseStatement() {
    if (isType(tok::TOK_KW)) {
        tok::KW_TYPE kw = as<tok::Keyword>()->getKwType();
        accept();

        switch (kw) {
        case tok::KW_DEF:   return parseDef();
        case tok::KW_IF:    return parseIf(true);
        default:            return nullptr;
        }

    } else if (accept(tok::OPER_L_BRACE)) {
        return parseBlock();
    } else {
        ExpressionStatement* expr = _mngr.New<ExpressionStatement>(parseExpression());
        expect(tok::OPER_SEMICOLON, "`;`");
        return expr;
    }
}

Expression* Parser::parseExpression() {
    return parseBinary(parsePrimary(), 0);
}

Expression* Parser::parseBinary(Expression* left, int precedence) {
    while (isType(tok::TOK_OPER)) {
        tok::Operator* oper = as<tok::Operator>();
        int newOpPrec = oper->getPrecedence();

        if (newOpPrec >= precedence) {

            if (Expression* expr = parseSpecialBinaryContinuity(left)) {
                left = expr;
                continue;
            } else {

                accept();
                Expression* right = parseBinary(parsePrimary(), newOpPrec);

                while (isType(tok::TOK_OPER)) {
                    if (as<tok::Operator>()->getPrecedence() > newOpPrec) {
                        right = parseBinary(right, as<tok::Operator>()->getPrecedence());
                    } else {
                        break;
                    }
                }

                left = _mngr.New<BinaryExpression>(left, right, _mngr.New<Identifier>(oper->toString()));
            }

        } else {
            break;
        }
    }

    return left;
}

Expression* Parser::parsePrimary() {
    Expression* toRet = nullptr;

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

    case tok::TOK_OPER:
        if (accept(tok::OPER_L_PAREN)) {
            Tuple* tuple = parseTuple();

            if (tuple->getExpressions().size() == 1) {
                return tuple->getExpressions()[0];
            } else {
                return tuple;
            }
        }
        else if (accept(tok::OPER_L_BRACE)) {
            return parseBlock();
        } else {
            _ctx->reporter().error(*_currentToken, "unexpected token `"+ _currentToken->toString() +"`");
        }
        break;

    case tok::TOK_KW:
        if (accept(tok::KW_IF)) {
            return parseIf(false);
        }
        break;

    default:
        _ctx->reporter().error(*_currentToken,
                               "expected int litteral | real litteral | string litteral "
                               "| identifier | keyword; got " + _currentToken->toString());
    }

    return toRet;
}

Block* Parser::parseBlock() {
    std::vector<Statement*> stats;

    while (!accept(tok::OPER_R_BRACE)) {
        stats.push_back(parseStatement());
    }

    return _mngr.New<Block>(stats);
}

IfExpression* Parser::parseIf(bool asStatement) {
    expect(tok::OPER_L_PAREN, "`(`");

    Expression* cond = parseExpression();

    expect(tok::OPER_R_PAREN, "`)`");

    ASTNode* then = asStatement ? (ASTNode*)parseStatement() : (ASTNode*)parseExpression();
    ASTNode* els = nullptr;

    if (accept(tok::KW_ELSE)) {
         els = asStatement ? (ASTNode*)parseStatement() : (ASTNode*)parseExpression();
    }

    return _mngr.New<IfExpression>(cond, then, els);
}

Expression* Parser::parseSpecialBinaryContinuity(Expression* left) {
    if (accept(tok::OPER_L_PAREN)) {
        return _mngr.New<FunctionCall>(left, parseTuple());
    } else if (accept(tok::OPER_FAT_ARROW)) {
        return _mngr.New<FunctionCreation>(static_cast<Tuple*>(left), parseExpression());
    } else if (accept(tok::OPER_DOT)) {
        return parseDotOperation(left);
    }

    return nullptr;
}

Tuple* Parser::parseTuple() {
    std::vector<Expression*> exprs;
    return parseTuple(exprs);
}

ast::Tuple* Parser::parseTuple(std::vector<ast::Expression*>& exprs) {

    if (!accept(tok::OPER_R_PAREN)) {
        do {
            if (Expression* arg = parseExpression()){
                exprs.push_back(arg);
            }
        } while (accept(tok::OPER_COMMA));

        expect(tok::OPER_R_PAREN, "`)`");
    }

    return _mngr.New<Tuple>(exprs);
}

Expression* Parser::parseDotOperation(Expression* left) {
    Identifier* ident = parseIdentifier("expected attribute / method name");

    if (accept(tok::OPER_L_PAREN)) {
        std::vector<Expression*> exprs(1);
        exprs[0] = left;
        return _mngr.New<FunctionCall>(ident, parseTuple(exprs));
    } else {
        return _mngr.New<MemberAccess>(left, ident);
    }

}



}
