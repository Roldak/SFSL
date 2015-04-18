//
//  Parser.cpp
//  SFSL
//
//  Created by Romain Beguet on 16.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Parser.h"

#include "../Lexer/Tokens/Litterals.h"
#include "../AST/Visitors/ASTTypeIdentifier.h"

#define SAVE_POS(ident) const common::Positionnable ident = *_currentToken;

namespace sfsl {

using namespace ast;

Parser::Parser(CompCtx_Ptr& ctx, lex::Lexer &lexer)
    : _ctx(ctx), _mngr(ctx->memoryManager()), _lex(lexer), _lastTokenEndPos(0), _currentToken(nullptr) {

}

Program* Parser::parse() {
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
    _lastTokenEndPos = _currentToken->getEndPosition();
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

Program* Parser::parseProgram() {
    std::vector<ModuleDecl*> modules;

    SAVE_POS(startPos)

    while (_lex.hasNext()) {
        expect(tok::KW_MODULE, "`module`", true);
        modules.push_back(parseModule());
    }

    Program* prog = _mngr.New<Program>(modules);
    prog->setPos(startPos);
    prog->setEndPos(_lastTokenEndPos);
    return prog;
}

ModuleDecl* Parser::parseModule() {

    Identifier* moduleName = parseIdentifier("expected module name");
    std::vector<ModuleDecl*> mods;
    std::vector<TypeDecl*> types;
    std::vector<DefineDecl*> decls;

    expect(tok::OPER_L_BRACE, "`{`");

    while (!accept(tok::OPER_R_BRACE)) {
        if (accept(tok::KW_MODULE)) {
            mods.push_back(parseModule());
        } else if (accept(tok::KW_TPE)) {
            types.push_back(parseType(false));
        } else if (accept(tok::KW_DEF)) {
            decls.push_back(parseDef(false));
        } else {
            expect(tok::OPER_R_BRACE, "`}`");
            break;
        }
    }

    ModuleDecl* modDecl = _mngr.New<ModuleDecl>(moduleName, mods, types, decls);
    modDecl->setPos(*moduleName);
    modDecl->setEndPos(_lastTokenEndPos);

    return modDecl;
}

DefineDecl* Parser::parseDef(bool asStatement) {

    Identifier* defName = parseIdentifier("expected definition name");

    if (!(isType(tok::TOK_OPER) && as<tok::Operator>()->getOpType() == tok::OPER_L_PAREN))
        expect(tok::OPER_EQ, "`=`");

    Expression* expr = parseExpression();

    if (asStatement) {
        expect(tok::OPER_SEMICOLON, "`;`");
    }

    DefineDecl* defDecl = _mngr.New<DefineDecl>(defName, expr);
    defDecl->setPos(*defName);
    defDecl->setEndPos(_lastTokenEndPos);
    return defDecl;

}

ClassDecl* Parser::parseClass() {
    SAVE_POS(startPos)

    std::string className = "<anonymous>";

    if (_currentToken->getTokenType() == tok::TOK_ID) {
        className = as<tok::Identifier>()->toString();
        accept();
    }

    expect(tok::OPER_L_BRACE, "`{`");

    std::vector<TypeSpecifier*> fields;
    std::vector<DefineDecl*> defs;

    while (!accept(tok::OPER_R_BRACE)) {
        if (accept(tok::KW_DEF)) {
            defs.push_back(parseDef(false));
        } else {
            Identifier* fieldName = parseIdentifier("expected field name | def");
            expect(tok::OPER_COLON, "`:`");
            Expression* type = parseExpression();
            expect(tok::OPER_SEMICOLON, "`;`");

            TypeSpecifier* field = _mngr.New<TypeSpecifier>(fieldName, type);
            field->setPos(*fieldName);
            field->setEndPos(_lastTokenEndPos);

            fields.push_back(field);
        }
    }

    ClassDecl* classDecl = _mngr.New<ClassDecl>(className, fields, defs);
    classDecl->setPos(startPos);
    classDecl->setEndPos(_lastTokenEndPos);
    return classDecl;
}

TypeDecl* Parser::parseType(bool asStatement) {
    Identifier* typeName = parseIdentifier("expected type name");
    accept(tok::OPER_EQ);

    Expression* expr = parseExpression();

    if (asStatement) {
        expect(tok::OPER_SEMICOLON, "`;`");
    }

    TypeDecl* typeDecl = _mngr.New<TypeDecl>(typeName, expr);
    typeDecl->setPos(*typeName);
    typeDecl->setEndPos(_lastTokenEndPos);
    return typeDecl;
}

Expression* Parser::parseStatement() {
    if (isType(tok::TOK_KW)) {
        tok::KW_TYPE kw = as<tok::Keyword>()->getKwType();
        accept();

        switch (kw) {
        case tok::KW_DEF:   return parseDef(true);
        case tok::KW_IF:    return parseIf(true);
        case tok::KW_TPE:   return parseType(true);
        default:            return nullptr;
        }
    } else if (accept(tok::OPER_L_BRACE)) {
        return parseBlock();
    } else {
        SAVE_POS(startPos)
        ExpressionStatement* expr = _mngr.New<ExpressionStatement>(parseExpression());
        expect(tok::OPER_SEMICOLON, "`;`");
        expr->setPos(startPos);
        expr->setEndPos(_lastTokenEndPos);
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

                if (left != nullptr) {
                    left = makeBinary(left, right, oper);
                }
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
        if (isType(tok::TOK_OPER) && as<tok::Operator>()->getOpType() == tok::OPER_COLON) {
            toRet = parseTypeSpecifier(static_cast<Identifier*>(toRet));
        }
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
        else if (accept(tok::OPER_L_BRACKET)) {
            return parseTypeTuple();
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
        } else if (accept(tok::KW_CLASS)) {
            return parseClass();
        }
        break;

    default:
        _ctx->reporter().error(*_currentToken,
                               "expected int litteral | real litteral | string litteral "
                               "| identifier | keyword; got " + _currentToken->toString());
    }

    return toRet;
}

TypeSpecifier* Parser::parseTypeSpecifier(Identifier* id) {
    Expression* expr = parseBinary(id, tok::Operator(tok::OPER_COLON).getPrecedence());
    TypeSpecifier* spec = _mngr.New<TypeSpecifier>(id, static_cast<BinaryExpression*>(expr)->getRhs());
    spec->setPos(*id);
    spec->setEndPos(_lastTokenEndPos);
    return spec;
}

Block* Parser::parseBlock() {
    std::vector<Expression*> stats;

    SAVE_POS(startPos)

    while (!accept(tok::OPER_R_BRACE)) {
        stats.push_back(parseStatement());
    }

    Block* block = _mngr.New<Block>(stats);
    block->setPos(startPos);
    block->setEndPos(_lastTokenEndPos);

    return block;
}

IfExpression* Parser::parseIf(bool asStatement) {
    SAVE_POS(startPos)

    Expression* cond = parseExpression();

    Expression* then = asStatement ? parseStatement() : parseExpression();
    Expression* els = nullptr;

    if (accept(tok::KW_ELSE)) {
         els = asStatement ? parseStatement() : parseExpression();
    }

    IfExpression* ifexpr = _mngr.New<IfExpression>(cond, then, els);
    ifexpr->setPos(startPos);
    ifexpr->setEndPos(_lastTokenEndPos);

    return ifexpr;
}

Expression* Parser::parseSpecialBinaryContinuity(Expression* left) {
    Expression* res = nullptr;

    if (accept(tok::OPER_L_PAREN)) {
        res = _mngr.New<FunctionCall>(left, parseTuple());
    } else if (accept(tok::OPER_FAT_ARROW)) {
        if (ast::isNodeOfType<TypeTuple>(left, _ctx)) {
            res = _mngr.New<TypeConstructorCreation>(static_cast<TypeTuple*>(left), parseExpression());
        } else {
            res = _mngr.New<FunctionCreation>(left, parseExpression());
        }
    } else if (accept(tok::OPER_DOT)) {
        return parseDotOperation(left);
    }

    if (res) {
        res->setPos(*left);
        res->setEndPos(_lastTokenEndPos);
    }

    return res;
}

Tuple* Parser::parseTuple() {
    std::vector<Expression*> exprs;
    return parseTuple<Tuple, tok::OPER_R_PAREN>(exprs);
}

TypeTuple *Parser::parseTypeTuple() {
    std::vector<Expression*> exprs;
    return parseTuple<TypeTuple, tok::OPER_R_BRACKET>(exprs);
}

Expression* Parser::parseDotOperation(Expression* left) {
    Identifier* ident = parseIdentifier("expected attribute / method name");
    MemberAccess* maccess = _mngr.New<MemberAccess>(left, ident);
    maccess->setPos(*left);
    maccess->setEndPos(_lastTokenEndPos);
    return maccess;
}

template<typename T, tok::OPER_TYPE R_DELIM>
T* Parser::parseTuple(std::vector<ast::Expression*>& exprs) {

    SAVE_POS(startPos)

    if (!accept(R_DELIM)) {
        do {
            if (Expression* arg = parseExpression()){
                exprs.push_back(arg);
            }
        } while (accept(tok::OPER_COMMA));

        expect(R_DELIM, "`)`");
    }

    T* tuple = _mngr.New<T>(exprs);
    tuple->setPos(startPos);
    tuple->setEndPos(_lastTokenEndPos);
    return tuple;
}

Expression* Parser::makeBinary(Expression* left, Expression* right, tok::Operator* oper) {
    Expression* res;

    switch (oper->getOpType()) {
    case tok::OPER_EQ:
        res = _mngr.New<AssignmentExpression>(left, right);
        break;
    default:
        res = _mngr.New<BinaryExpression>(left, right, _mngr.New<Identifier>(oper->toString()));
        break;
    }

    res->setPos(*left);
    res->setEndPos(_lastTokenEndPos);
    return res;
}

}
