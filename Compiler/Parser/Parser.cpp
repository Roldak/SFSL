//
//  Parser.cpp
//  SFSL
//
//  Created by Romain Beguet on 16.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Parser.h"

#include "../Lexer/Tokens/Litterals.h"

#define SAVE_POS(ident) const common::Positionnable ident = *_currentToken;

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

    SAVE_POS(startPos)

    if (isType(tok::TOK_ID)) {
        name = as<tok::Identifier>()->toString();
        accept();
    } else {
        _ctx->reporter().error(*_currentToken, errMsg);
    }

    Identifier* id = _mngr.New<Identifier>(name);
    id->setPos(startPos);
    return id;
}

// Parsing

ASTNode* Parser::parseProgram() {
    std::vector<ModuleDecl*> modules;

    SAVE_POS(startPos)

    while (_lex.hasNext()) {
        expect(tok::KW_MODULE, "`module`", true);
        modules.push_back(parseModule());
    }

    Program* prog = _mngr.New<Program>(modules);
    prog->setPos(startPos);
    return prog;
}

ModuleDecl* Parser::parseModule() {

    Identifier* moduleName = parseIdentifier("expected module name");
    std::vector<ModuleDecl*> mods;
    std::vector<DefineDecl*> decls;

    expect(tok::OPER_L_BRACE, "`{`");

    while (!accept(tok::OPER_R_BRACE)) {
        if (accept(tok::KW_MODULE)) {
            mods.push_back(parseModule());
        } else if (accept(tok::KW_DEF)) {
            decls.push_back(parseDef(false));
        } else {
            expect(tok::OPER_R_BRACE, "`}`");
            break;
        }
    }

    ModuleDecl* modDecl = _mngr.New<ModuleDecl>(moduleName, mods, decls);
    modDecl->setPos(*moduleName);
    return modDecl;
}

DefineDecl* Parser::parseDef(bool asStatement) {

    Identifier* defName = parseIdentifier("expected definition name");

    if (!(isType(tok::TOK_OPER) && as<tok::Operator>()->getOpType() == tok::OPER_L_PAREN))
        expect(tok::OPER_EQ, "`=`");

    Expression* stat = parseExpression();

    if (asStatement) {
        expect(tok::OPER_SEMICOLON, "`;`");
    }

    DefineDecl* defDecl = _mngr.New<DefineDecl>(defName, stat);
    defDecl->setPos(*defName);
    return defDecl;

}

Statement* Parser::parseStatement() {
    if (isType(tok::TOK_KW)) {
        tok::KW_TYPE kw = as<tok::Keyword>()->getKwType();
        accept();

        switch (kw) {
        case tok::KW_DEF:   return parseDef(true);
        case tok::KW_IF:    return parseIf(true);
        default:            return nullptr;
        }

    } else if (accept(tok::OPER_L_BRACE)) {
        return parseBlock();
    } else {
        SAVE_POS(startPos)
        ExpressionStatement* expr = _mngr.New<ExpressionStatement>(parseExpression());
        expr->setPos(startPos);
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

                common::Positionnable& leftPos = *left;
                left = _mngr.New<BinaryExpression>(left, right, _mngr.New<Identifier>(oper->toString()));
                left->setPos(leftPos);
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
        toRet = _mngr.New<IntLitteral>(as<tok::IntLitteral>()->getValue());
        toRet->setPos(*_currentToken);
        accept();
        break;

    case tok::TOK_REAL_LIT:
        toRet = _mngr.New<RealLitteral>(as<tok::RealLitteral>()->getValue());
        toRet->setPos(*_currentToken);
        accept();
        break;

    case tok::TOK_STR_LIT:
        break;
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

    SAVE_POS(startPos)

    while (!accept(tok::OPER_R_BRACE)) {
        stats.push_back(parseStatement());
    }
    Block* block = _mngr.New<Block>(stats);
    block->setPos(startPos);
    return block;
}

IfExpression* Parser::parseIf(bool asStatement) {
    SAVE_POS(startPos)

    expect(tok::OPER_L_PAREN, "`(`");

    Expression* cond = parseExpression();

    expect(tok::OPER_R_PAREN, "`)`");

    ASTNode* then = asStatement ? (ASTNode*)parseStatement() : (ASTNode*)parseExpression();
    ASTNode* els = nullptr;

    if (accept(tok::KW_ELSE)) {
         els = asStatement ? (ASTNode*)parseStatement() : (ASTNode*)parseExpression();
    }

    IfExpression* ifexpr = _mngr.New<IfExpression>(cond, then, els);
    ifexpr->setPos(startPos);
    return ifexpr;
}

Expression* Parser::parseSpecialBinaryContinuity(Expression* left) {
    if (accept(tok::OPER_L_PAREN)) {
        FunctionCall* fcall = _mngr.New<FunctionCall>(left, parseTuple());
        fcall->setPos(*left);
        return fcall;
    } else if (accept(tok::OPER_FAT_ARROW)) {
        FunctionCreation* fcall = _mngr.New<FunctionCreation>(left, parseExpression());
        fcall->setPos(*left);
        return fcall;
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

    SAVE_POS(startPos)

    if (!accept(tok::OPER_R_PAREN)) {
        do {
            if (Expression* arg = parseExpression()){
                exprs.push_back(arg);
            }
        } while (accept(tok::OPER_COMMA));

        expect(tok::OPER_R_PAREN, "`)`");
    }

    Tuple* tuple = _mngr.New<Tuple>(exprs);
    tuple->setPos(startPos);
    return tuple;
}

Expression* Parser::parseDotOperation(Expression* left) {
    Identifier* ident = parseIdentifier("expected attribute / method name");

    if (accept(tok::OPER_L_PAREN)) {
        std::vector<Expression*> exprs(1);
        exprs[0] = left;
        return _mngr.New<FunctionCall>(ident, parseTuple(exprs));
    } else {
        MemberAccess* maccess = _mngr.New<MemberAccess>(left, ident);
        maccess->setPos(*ident);
        return maccess;
    }

}



}
