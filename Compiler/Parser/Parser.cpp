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

const std::string Parser::AnonymousClassName = "<anonymous class>";
const std::string Parser::AnonymousTypeConstructorName = "<anonymous type constructor>";
const std::string Parser::AnonymousFunctionName = "<anonymous function>";

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

template<typename T>
T* Parser::parseIdentifierHelper(const std::string errMsg) {
    std::string name;

    SAVE_POS(startPos)

    if (isType(tok::TOK_ID)) {
        name = as<tok::Identifier>()->toString();
        accept();
    } else {
        _ctx->reporter().error(*_currentToken, errMsg);
    }

    T* id = _mngr.New<T>(name);
    id->setPos(startPos);
    return id;
}

Identifier* Parser::parseIdentifier(const std::string& errMsg) {
    return parseIdentifierHelper<Identifier>(errMsg);
}

TypeIdentifier *Parser::parseTypeIdentifier(const std::string &errMsg) {
    return parseIdentifierHelper<TypeIdentifier>(errMsg);
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

    std::string className = _currentTypeName.empty() ? AnonymousClassName : _currentTypeName;
    TypeExpression* parent = nullptr;

    if (_currentToken->getTokenType() == tok::TOK_ID) {
        className = as<tok::Identifier>()->toString();
        accept();
    }

    if (accept(tok::OPER_COLON)) {
        parent = parseTypeExpression();
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
            TypeExpression* type = parseTypeExpression();
            expect(tok::OPER_SEMICOLON, "`;`");

            TypeSpecifier* field = _mngr.New<TypeSpecifier>(fieldName, type);
            field->setPos(*fieldName);
            field->setEndPos(_lastTokenEndPos);

            fields.push_back(field);
        }
    }

    ClassDecl* classDecl = _mngr.New<ClassDecl>(className, parent, fields, defs);
    classDecl->setPos(startPos);
    classDecl->setEndPos(_lastTokenEndPos);
    return classDecl;
}

TypeDecl* Parser::parseType(bool asStatement) {
    TypeIdentifier* typeName = parseTypeIdentifier("expected type name");
    accept(tok::OPER_EQ);

    std::string lastTypeName = _currentTypeName;
    _currentTypeName = typeName->getValue();

    TypeExpression* expr = parseTypeExpression();

    _currentTypeName = lastTypeName;

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
        if (accept(tok::OPER_COLON)) {
            toRet = parseTypeSpecifier(static_cast<Identifier*>(toRet));
        }
        break;

    case tok::TOK_OPER:
        if (accept(tok::OPER_L_PAREN)) {
            Tuple* tuple = static_cast<Tuple*>(toRet = parseTuple());

            if (tuple->getExpressions().size() == 1) {
                toRet = tuple->getExpressions()[0];
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
        } else {
            _ctx->reporter().error(*_currentToken, "unexpected keyword `" + _currentToken->toString() + "`");
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
    TypeExpression* expr = parseTypeExpression();
    TypeSpecifier* spec = _mngr.New<TypeSpecifier>(id, expr);
    spec->setPos(*id);
    spec->setEndPos(_lastTokenEndPos);
    return spec;
}

TypeExpression* Parser::parseTypeExpression() {
    return parseTypeBinary(parseTypePrimary(), 0);
}

TypeExpression* Parser::parseTypeBinary(TypeExpression* left, int precedence) {
    while (isType(tok::TOK_OPER)) {
        tok::Operator* op = as<tok::Operator>();
        int newOpPrec = op->getPrecedence();

        if (newOpPrec >= precedence) {

            SAVE_POS(startPos)

            TypeExpression* expr;

            accept();

            switch (op->getOpType()) {
            case tok::OPER_L_BRACKET:
                expr = _mngr.New<TypeConstructorCall>(left, parseTypeTuple());
                break;
            case tok::OPER_FAT_ARROW:
                expr = makeFuncOrTypeConstr<TypeConstructorCreation>(left, [&](){return parseTypeExpression();});
                break;
            case tok::OPER_DOT:
                expr = _mngr.New<TypeMemberAccess>(left, parseTypeIdentifier("Expected member name"));
                break;
            default:
                _ctx->reporter().error(*op, "Unexpected operator `" + op->toString() + "`");
                continue;
            }

            expr->setPos(startPos);
            expr->setEndPos(_lastTokenEndPos);

            left = expr;

        } else {
            break;
        }
    }

    return left;
}

TypeExpression* Parser::parseTypePrimary() {
    TypeExpression* toRet = nullptr;

    switch (_currentToken->getTokenType()) {
    case tok::TOK_ID:
        toRet = parseTypeIdentifier();
        if (isType(tok::TOK_OPER) && as<tok::Operator>()->getOpType() == tok::OPER_COLON) {
            //toRet = parseTypeSpecifier(static_cast<Identifier*>(toRet));
        }
        break;

    case tok::TOK_OPER:
        if (accept(tok::OPER_L_BRACKET)) {
            TypeTuple* ttuple = static_cast<TypeTuple*>(toRet = parseTypeTuple());

            if (ttuple->getExpressions().size() == 1) {
                //toRet = ttuple->getExpressions()[0];
            }
        }
        else {
            _ctx->reporter().error(*_currentToken, "Unexpected token `"+ _currentToken->toString() +"`");
        }
        break;

    case tok::TOK_KW:
        if (accept(tok::KW_CLASS)) {
            return parseClass();
        } else {
            _ctx->reporter().error(*_currentToken, "Unexpected keyword `" + _currentToken->toString() + "`");
        }
        break;

    default:
        _ctx->reporter().error(*_currentToken,
                               "expected identifier | type tuple | class "
                               "; got " + _currentToken->toString());
    }

    return toRet;
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
        res = makeFuncOrTypeConstr<FunctionCreation>(left, [&](){return parseExpression();});
    } else if (accept(tok::OPER_DOT)) {
        res = _mngr.New<MemberAccess>(left, parseIdentifier("Expected attribute / method name"));
    } // no match is not an error

    if (res) {
        res->setPos(*left);
        res->setEndPos(_lastTokenEndPos);
    }

    return res;
}

Tuple* Parser::parseTuple() {
    std::vector<Expression*> exprs;
    return parseTuple<Tuple, tok::OPER_R_PAREN, Expression>(exprs, [&](){return parseExpression();});
}

TypeTuple *Parser::parseTypeTuple() {
    std::vector<TypeExpression*> exprs;
    return parseTuple<TypeTuple, tok::OPER_R_BRACKET, TypeExpression>(exprs, [&](){return parseTypeExpression();});
}

template<typename RETURN_TYPE, tok::OPER_TYPE R_DELIM, typename ELEMENT_TYPE, typename PARSING_FUNC>
RETURN_TYPE* Parser::parseTuple(std::vector<ELEMENT_TYPE*>& exprs, const PARSING_FUNC& f) {

    SAVE_POS(startPos)

    if (!accept(R_DELIM)) {
        do {
            if (ELEMENT_TYPE* arg = f()){
                exprs.push_back(arg);
            }
        } while (accept(tok::OPER_COMMA));

        expect(R_DELIM, "`" + tok::Operator::OperTypeToString(R_DELIM) + "`");
    }

    RETURN_TYPE* tuple = _mngr.New<RETURN_TYPE>(exprs);
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

template<typename RETURN_TYPE, typename EXPRESSION_TYPE, typename PARSING_FUNC>
RETURN_TYPE *Parser::makeFuncOrTypeConstr(EXPRESSION_TYPE* left, const PARSING_FUNC& f) {
    std::string name = _currentDefName.empty() ? AnonymousFunctionName : _currentDefName;
    return _mngr.New<RETURN_TYPE>(name, left, f());
}

}
