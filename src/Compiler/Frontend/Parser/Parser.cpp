//
//  Parser.cpp
//  SFSL
//
//  Created by Romain Beguet on 16.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Parser.h"

#include <functional>

#include "../Lexer/Tokens.h"
#include "../AST/Visitors/ASTTypeIdentifier.h"
#include "../AST/Visitors/ASTExpr2TypeExpr.h"

#define SAVE_POS(ident) const common::Positionnable ident = *_currentToken;

namespace sfsl {

using namespace ast;

Parser::Parser(CompCtx_Ptr& ctx, lex::Lexer& lexer, const common::AbstractPrimitiveNamer* namer)
    : _ctx(ctx), _mngr(ctx->memoryManager()), _lex(lexer), _namer(namer), _lastTokenEndPos(0), _currentToken(nullptr) {

}

Program* Parser::parse() {
    _currentToken = _lex.getNext();
    return parseProgram();
}

Expression* Parser::parseSingleExpression() {
    _currentToken = _lex.getNext();
    return parseExpression();
}

TypeExpression* Parser::parseType() {
    _currentToken = _lex.getNext();
    return parseTypeExpression();
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

void Parser::reportUnexpectedCurrentToken() {
    _ctx->reporter().error(*_currentToken, "Unexpected token `" + _currentToken->toString() + "`");
}

template<typename T>
bool Parser::expect(T type, const std::string& expected, bool fatal) {
   tok::Token* lastTok = _currentToken;

   if (!accept(type)) {
       if (fatal) {
           _ctx->reporter().fatal(*lastTok, "Expected " + expected + " but got `" + lastTok->toString() + "`");
       } else {
           _ctx->reporter().error(*lastTok, "Expected " + expected + " but got `" + lastTok->toString() + "`");
       }
       return false;
   }

   return true;
}

template<typename T>
T Parser::expectSemicolonAndReturn(T expr) {
    expect(tok::OPER_SEMICOLON, "`;`");
    return expr;
}

template<typename T>
T* Parser::as() {
    return static_cast<T*>(_currentToken);
}

template<typename T>
T* Parser::parseIdentifierHelper(const std::string& errMsg) {
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

TypeIdentifier* Parser::parseTypeIdentifier(const std::string& errMsg) {
    return parseIdentifierHelper<TypeIdentifier>(errMsg);
}

DefFlags consumeDefFlags(DefFlags& flags, DefFlags toConsume) {
    DefFlags val = flags & toConsume;
    flags &= ~toConsume;
    return val;
}

// Parsing

Program* Parser::parseProgram() {
    std::vector<ModuleDecl*> modules;

    SAVE_POS(startPos)

    while (_lex.hasNext()) {
        parseAnnotations();

        expect(tok::KW_MODULE, "`module`", true);
        modules.push_back(parseModule());

        reportErroneousAnnotations();
    }

    Program* prog = _mngr.New<Program>(modules);
    prog->setPos(startPos);
    prog->setEndPos(_lastTokenEndPos);
    return prog;
}

ModuleDecl* Parser::parseModule() {
    Identifier* moduleName = parseIdentifier("Expected module name");
    std::vector<ModuleDecl*> mods;
    std::vector<TypeDecl*> types;
    std::vector<DefineDecl*> decls;
    std::vector<CanUseModules::ModulePath> usings;
    std::vector<Annotation*> annots(std::move(consumeAnnotations()));

    expect(tok::OPER_L_BRACE, "`{`");

    while (!accept(tok::OPER_R_BRACE) && !accept(tok::TOK_EOF)) {
        parseAnnotations();

        SAVE_POS(keywordPos)

        DefFlags flags = parseDefFlags();

        SAVE_POS(externElemPos)

        if (accept(tok::KW_MODULE)) {
            mods.push_back(parseModule());
        } else if (accept(tok::KW_TPE)) {
            types.push_back(parseTypeDecl(consumeDefFlags(flags, DefFlags::EXTERN)));
        } else if (accept(tok::KW_CLASS)) {
            types.push_back(desugarTopLevelClassDecl(consumeDefFlags(flags, DefFlags::EXTERN | DefFlags::ABSTRACT)));
        } else if (accept(tok::KW_DEF)) {
            decls.push_back(parseDef(consumeDefFlags(flags, DefFlags::EXTERN)));
        } else if (accept(tok::KW_USING)) {
            usings.push_back(parseUsing(keywordPos, false));
        } else {
            expect(tok::OPER_R_BRACE, "`}`");
            break;
        }

        if (flags % DefFlags::EXTERN) {
            _ctx->reporter().error(externElemPos, "Modules or type declarations cannot be declared extern");
        }
        if (flags % DefFlags::ABSTRACT) {
            _ctx->reporter().error(externElemPos, "Only classes can be declared abstract");
        }
        if (flags % DefFlags::STATIC) {
            _ctx->reporter().error(externElemPos, "Static is not a valid flag in this context");
        }

        reportErroneousAnnotations();
    }

    ModuleDecl* modDecl = _mngr.New<ModuleDecl>(moduleName, mods, types, decls);
    modDecl->setUsedModules(usings);
    modDecl->setAnnotations(annots);
    modDecl->setPos(*moduleName);
    modDecl->setEndPos(_lastTokenEndPos);

    return modDecl;
}

DefineDecl* Parser::parseDef(DefFlags flags, Identifier* name) {
    std::vector<Annotation*> annots(std::move(consumeAnnotations()));

    Identifier* defName = (name ? name : parseIdentifier("Expected definition name"));

    TypeExpression* typeSpecifier = nullptr;
    Expression* expr = nullptr;

    if (flags % DefFlags::EXTERN || flags % DefFlags::ABSTRACT) {
        // Case: extern def f: int->int
        expect(tok::OPER_COLON, "`:`");
        typeSpecifier = parseTypeExpression();
    } else if (accept(tok::OPER_COLON)) {
        // Case: def f: int->int = (x: int) => 2 * x
        typeSpecifier = parseTypeExpression();
        expect(tok::OPER_EQ, "`=`");
        expr = parseExpression();
    } else if (isType(tok::TOK_OPER) && (as<tok::Operator>()->getOpType() == tok::OPER_L_PAREN ||
                                         as<tok::Operator>()->getOpType() == tok::OPER_L_BRACKET)) {
        expr = parseExpression();
    } else {
        expect(tok::OPER_EQ, "`=`");
        expr = parseExpression();
    }

    DefineDecl* defDecl = _mngr.New<DefineDecl>(defName, typeSpecifier, expr, flags);
    defDecl->setAnnotations(annots);
    defDecl->setPos(*defName);
    defDecl->setEndPos(_lastTokenEndPos);
    return defDecl;
}

ClassDecl* Parser::parseClass(bool isAbstract) {
    SAVE_POS(startPos)

    std::string className;
    if (_currentToken->getTokenType() == tok::TOK_ID) {
        className = as<tok::Identifier>()->toString();
        accept();
    } else {
        className = _currentTypeName.empty() ? AnonymousClassName : _currentTypeName;
    }

    return parseClassBody(isAbstract, className, startPos);
}

TypeDecl* Parser::parseTypeDecl(DefFlags flags) {
    std::vector<Annotation*> annots(std::move(consumeAnnotations()));

    TypeIdentifier* typeName = parseTypeIdentifier("Expected type name");
    accept(tok::OPER_EQ);

    std::string lastTypeName = _currentTypeName;
    _currentTypeName = typeName->getValue();

    TypeExpression* expr = parseTypeExpression();

    _currentTypeName = lastTypeName;

    TypeDecl* typeDecl = _mngr.New<TypeDecl>(typeName, expr, flags % DefFlags::EXTERN);
    typeDecl->setAnnotations(annots);
    typeDecl->setPos(*typeName);
    typeDecl->setEndPos(_lastTokenEndPos);
    return typeDecl;
}

Expression* Parser::parseStatement() {
    SAVE_POS(startPos)

    parseAnnotations();

    if (isType(tok::TOK_KW)) {
        tok::KW_TYPE kw = as<tok::Keyword>()->getKwType();
        accept();

        switch (kw) {
        case tok::KW_DEF:   return expectSemicolonAndReturn(parseDef(DefFlags::NONE));
        case tok::KW_TPE:   return expectSemicolonAndReturn(parseTypeDecl(DefFlags::NONE));
        case tok::KW_CLASS: return expectSemicolonAndReturn(desugarTopLevelClassDecl(DefFlags::NONE));
        case tok::KW_IF:    reportErroneousAnnotations(); return parseIf(true);
        case tok::KW_THIS:  return expectSemicolonAndReturn(makeThis(startPos));
        case tok::KW_NEW:   return expectSemicolonAndReturn(parseNew(startPos));

        case tok::KW_ABSTRACT:
            if (accept(tok::KW_CLASS)) {
                return expectSemicolonAndReturn(desugarTopLevelClassDecl(DefFlags::ABSTRACT));
            }
            _ctx->reporter().error(startPos, "Expected `class` keyword after `abstract` flag");
            return nullptr;

        case tok::KW_REDEF:
        case tok::KW_STATIC:
            reportErroneousAnnotations();
            _ctx->reporter().error(startPos, "`" + tok::Keyword::KeywordTypeToString(kw) +
                                   "` keyword can only be used inside a class scope");
            return nullptr;
        default:
            reportErroneousAnnotations();
            _ctx->reporter().error(startPos, "Unexpected keyword `" + tok::Keyword::KeywordTypeToString(kw) + "`");
            return nullptr;
        }
    } else if (accept(tok::OPER_L_BRACE)) {
        reportErroneousAnnotations();
        return parseBlock();
    } else {
        reportErroneousAnnotations();
        ExpressionStatement* expr = _mngr.New<ExpressionStatement>(expectSemicolonAndReturn(parseExpression()));
        //Expression* expr = expectSemicolonAndReturn(parseExpression());
        expr->setPos(startPos);
        expr->setEndPos(_lastTokenEndPos);
        return expr;
    }
}

Expression* Parser::parseExpression() {
    return parseBinary(parsePrimary(), 0);
}

Expression* Parser::parseUnary() {
    tok::OPER_TYPE oper = as<tok::Operator>()->getOpType();

    if (oper == tok::OPER_MINUS || oper == tok::OPER_PLUS || oper == tok::OPER_BANG || oper == tok::OPER_TILDE) {
        SAVE_POS(pos)
        std::string operName = as<tok::Operator>()->toString();
        accept();

        Expression* callee = parseBinary(parsePrimary(), tok::Operator::getUnaryOperatorPrecedence());
        Expression* toRet = makeMethodCall(callee, operName, {}, *callee, *callee);

        toRet->setPos(pos);
        toRet->setEndPos(_lastTokenEndPos);

        return toRet;
    }

    return nullptr;
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
                Expression* right = parsePrimary();

                while (isType(tok::TOK_OPER)) {
                    int curOpPrec = as<tok::Operator>()->getPrecedence();
                    if (curOpPrec > newOpPrec || (curOpPrec == newOpPrec && as<tok::Operator>()->isRightAssociative())) {
                        right = parseBinary(right, curOpPrec);
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
    SAVE_POS(startPos)
    Expression* toRet = nullptr;
    bool shouldReportAnnotations = true;

    parseAnnotations();

    switch (_currentToken->getTokenType()) {
    case tok::TOK_BOOL_LIT:
        toRet = _mngr.New<BoolLiteral>(as<tok::BoolLiteral>()->getValue());
        toRet->setPos(*_currentToken);
        accept();
        break;

    case tok::TOK_INT_LIT:
        toRet = _mngr.New<IntLiteral>(as<tok::IntLiteral>()->getValue());
        toRet->setPos(*_currentToken);
        accept();
        break;

    case tok::TOK_REAL_LIT:
        toRet = _mngr.New<RealLiteral>(as<tok::RealLiteral>()->getValue());
        toRet->setPos(*_currentToken);
        accept();
        break;

    case tok::TOK_STR_LIT:
        toRet = _mngr.New<StringLiteral>(as<tok::StringLiteral>()->getValue());
        toRet->setPos(*_currentToken);
        accept();
        break;

    case tok::TOK_ID:
        toRet = parseIdentifier();
        if (accept(tok::OPER_COLON)) {
            toRet = parseTypeSpecifier(static_cast<Identifier*>(toRet));
        }
        break;

    case tok::TOK_OPER:
        if (accept(tok::OPER_L_PAREN)) {
            std::vector<Annotation*> annots(std::move(consumeAnnotations()));
            Tuple* tuple = static_cast<Tuple*>(toRet = parseTuple());
            _parsedAnnotations = std::move(annots);

            if (tuple->getExpressions().size() == 1) {
                toRet = tuple->getExpressions()[0];
            }

            shouldReportAnnotations = false;
        }
        else if (accept(tok::OPER_L_BRACKET)) {
            std::vector<Annotation*> annots(std::move(consumeAnnotations()));

            TypeTuple* typeParams = parseTypeParameters();

            expect(tok::OPER_L_PAREN, "`(`");
            Tuple* args = parseTuple();

            TypeExpression* retType = nullptr;
            if (accept(tok::OPER_THIN_ARROW)) {
                retType = parseTypeExpression(false);
            }

            expect(tok::OPER_FAT_ARROW, "`=>`");
            toRet = _mngr.New<FunctionCreation>(_currentDefName.empty() ? AnonymousFunctionName : _currentDefName,
                                               typeParams, args, parseExpression(), retType);
            static_cast<FunctionCreation*>(toRet)->setAnnotations(annots);
            toRet->setPos(startPos);
            toRet->setEndPos(_lastTokenEndPos);
        }
        else if (accept(tok::OPER_L_BRACE)) {
            toRet = parseBlock();
        }
        else if (accept(tok::OPER_FAT_ARROW)) {
            std::vector<Annotation*> annots(std::move(consumeAnnotations()));

            Tuple* args = _mngr.New<Tuple>(std::vector<Expression*>());
            args->setPos(startPos);
            toRet = _mngr.New<FunctionCreation>(_currentDefName.empty() ? AnonymousFunctionName : _currentDefName,
                                               nullptr, args, parseExpression(), nullptr);

            static_cast<FunctionCreation*>(toRet)->setAnnotations(annots);

            toRet->setPos(startPos);
            toRet->setEndPos(_lastTokenEndPos);
        }
        else if (!(toRet = parseUnary())) {
            reportUnexpectedCurrentToken();
            accept();
        }
        break;

    case tok::TOK_KW:
        if (accept(tok::KW_IF)) {
            toRet = parseIf(false);
        } else if (accept(tok::KW_THIS)) {
            toRet = makeThis(startPos);
        } else if (accept(tok::KW_NEW)) {
            toRet = parseNew(startPos);
        } else {
            _ctx->reporter().error(*_currentToken, "Unexpected keyword `" + _currentToken->toString() + "`");
            accept();
        }
        break;

    default:
        _ctx->reporter().error(*_currentToken,
                               "Expected int Literal | real Literal | string Literal "
                               "| identifier | keyword; got " + _currentToken->toString());
        accept();
    }

    if (shouldReportAnnotations) {
        reportErroneousAnnotations();
    }

    return toRet;
}

TypeSpecifier* Parser::parseTypeSpecifier(Identifier* id) {
    TypeExpression* expr;

    if (_currentToken->getTokenType() == tok::TOK_OPER && as<tok::Operator>()->getOpType() == tok::OPER_EQ) {
        expr = _mngr.New<TypeToBeInferred>();
        expr->setPos(*_currentToken);
    } else {
        expr = parseTypeExpression();
    }

    TypeSpecifier* spec = _mngr.New<TypeSpecifier>(id, expr);
    spec->setPos(*id);
    spec->setEndPos(_lastTokenEndPos);
    return spec;
}

Block* Parser::parseBlock() {
    std::vector<Expression*> stats;
    std::vector<CanUseModules::ModulePath> usings;

    size_t leftBracePos = _lastTokenEndPos - 1;
    SAVE_POS(startPos)

    while (!accept(tok::OPER_R_BRACE) && !accept(tok::TOK_EOF)) {
        SAVE_POS(usingPos)

        if (accept(tok::KW_USING)) {
            usings.push_back(parseUsing(usingPos, true));
        } else {
            stats.push_back(parseStatement());
        }
    }

    Block* block = _mngr.New<Block>(stats);
    block->setUsedModules(usings);
    block->setPos(startPos);
    block->setStartPos(leftBracePos);
    block->setEndPos(_lastTokenEndPos);

    return block;
}

IfExpression* Parser::parseIf(bool asStatement) {
    SAVE_POS(startPos)

    expect(tok::OPER_L_PAREN, "`(`");

    Expression* cond = parseExpression();

    expect(tok::OPER_R_PAREN, "`)`");

    Expression* then = asStatement ? parseStatement() : parseExpression();
    Expression* els = nullptr;

    if (accept(tok::KW_ELSE)) {
         els = asStatement ? parseStatement() : parseExpression();
    }

    IfExpression* ifexpr = _mngr.New<IfExpression>(cond, then, els, false);
    ifexpr->setPos(startPos);
    ifexpr->setEndPos(_lastTokenEndPos);

    return ifexpr;
}

Expression* Parser::parseSpecialBinaryContinuity(Expression* left) {
    Expression* res = nullptr;

    SAVE_POS(fallbackPos)

    if (accept(tok::OPER_L_PAREN)) {
        res = _mngr.New<FunctionCall>(left, nullptr, parseTuple());
    }
    else if (accept(tok::OPER_L_BRACKET)) {
        TypeTuple* typeArgs = parseTypeTuple();
        if (accept(tok::OPER_L_PAREN)) {
            res = _mngr.New<FunctionCall>(left, typeArgs, parseTuple());
        } else if (TypeExpression* typeLeft = ast::ASTExpr2TypeExpr::convert(left, _ctx)) {
            res = parseTypeBinary(_mngr.New<TypeConstructorCall>(typeLeft, typeArgs), tok::Operator(tok::OPER_L_BRACKET).getPrecedence(), true);
        } else {
            reportUnexpectedCurrentToken();
        }
    }
    else if (accept(tok::OPER_FAT_ARROW)) {
        std::vector<Annotation*> annots(std::move(consumeAnnotations()));
        res = _mngr.New<FunctionCreation>(
                    _currentDefName.empty() ? AnonymousFunctionName : _currentDefName,
                    nullptr, left, parseExpression());
        static_cast<FunctionCreation*>(res)->setAnnotations(annots);
    }
    else if (accept(tok::OPER_THIN_ARROW)) {
        std::vector<Annotation*> annots(std::move(consumeAnnotations()));
        TypeExpression* retType = parseTypeExpression(false);
        expect(tok::OPER_FAT_ARROW, "`=>`");
        res = _mngr.New<FunctionCreation>(
                    _currentDefName.empty() ? AnonymousFunctionName : _currentDefName,
                    nullptr, left, parseExpression(), retType);
        static_cast<FunctionCreation*>(res)->setAnnotations(annots);
    }
    else if (accept(tok::OPER_DOT)) {
        Identifier* id = isType(tok::TOK_OPER) ? parseOperatorsAsIdentifer() : parseIdentifier("Expected attribute / method name");
        res = _mngr.New<MemberAccess>(left, id);
    } // no match is not an error

    if (res) {
        res->setPos(left ? *left : fallbackPos);
        res->setEndPos(_lastTokenEndPos);
    }

    reportErroneousAnnotations();

    return res;
}

Tuple* Parser::parseTuple() {
    std::vector<Expression*> exprs;
    return parseTuple<Tuple, tok::OPER_R_PAREN, Expression*>(exprs, [&](){return parseExpression();});
}

TypeExpression* Parser::parseTypeExpression(bool allowTypeConstructor) {
    return parseTypeBinary(parseTypePrimary(allowTypeConstructor), 0, allowTypeConstructor);
}

TypeExpression* Parser::parseTypeBinary(TypeExpression* left, int precedence, bool allowTypeConstructor) {
    if (!left) {
        return nullptr;
    }

    while (isType(tok::TOK_OPER)) {
        tok::Operator* op = as<tok::Operator>();
        int newOpPrec = op->getPrecedence();

        if (newOpPrec >= precedence && op->getOpType() != tok::OPER_EQ && op->getOpType() != tok::OPER_LT) {

            TypeExpression* expr;

            switch (op->getOpType()) {
            case tok::OPER_L_BRACKET:
                accept();
                expr = _mngr.New<TypeConstructorCall>(left, parseTypeTuple());
                break;
            case tok::OPER_FAT_ARROW:
                if (!allowTypeConstructor) {
                    return left;
                }

                accept();
                expr = _mngr.New<TypeConstructorCreation>(
                            _currentTypeName.empty() ? AnonymousTypeConstructorName : _currentTypeName,
                            left, parseTypeExpression());
                break;
            case tok::OPER_DOT:
                accept();
                expr = _mngr.New<TypeMemberAccess>(left, parseTypeIdentifier("Expected member name"));
                break;
            default:
                accept();
                _ctx->reporter().error(*op, "Unexpected operator `" + op->toString() + "`");
                continue;
            }

            expr->setPos(*left);
            expr->setEndPos(_lastTokenEndPos);

            left = expr;

        } else {
            break;
        }
    }

    return left;
}

TypeExpression* Parser::createFunctionTypeDecl(const TypeTuple* typeParams, const std::vector<TypeExpression*>& args, TypeExpression* ret) {
    std::vector<TypeExpression*> params;
    if (typeParams) {
        params = typeParams->getExpressions();
    }
    return FunctionTypeDecl::make(params, args, ret, _namer->Func(args.size()), _ctx);
}

TypeTuple* Parser::parseTypeTuple() {
    std::vector<TypeExpression*> exprs;
    return parseTuple<TypeTuple, tok::OPER_R_BRACKET, TypeExpression*>(exprs, [&](){return parseTypeExpression();});
}

TypeTuple* Parser::parseTypeParameters() {
    std::vector<TypeExpression*> typeParameters;

    SAVE_POS(startPos)

    if (!accept(tok::OPER_R_BRACKET)) {
        do {
            SAVE_POS(tpStartPos)

            common::VARIANCE_TYPE vt;
            TypeIdentifier* ident;
            KindSpecifyingExpression* kindExpr;

            if (accept(tok::KW_IN)) {
                vt = common::VAR_T_IN;
            } else if (accept(tok::KW_OUT)) {
                vt = common::VAR_T_OUT;
            } else {
                vt = common::VAR_T_NONE;
            }

            ident = parseTypeIdentifier("Expected type name");

            if (accept(tok::OPER_COLON)) {
                kindExpr = parseKindSpecifyingExpression();
            } else {
                TypeExpression* lb = accept(tok::OPER_GT) ? parseTypeExpression(false) : nullptr;
                TypeExpression* ub = accept(tok::OPER_LT) ? parseTypeExpression(false) : nullptr;

                kindExpr = _mngr.New<ProperTypeKindSpecifier>(lb, ub);
                kindExpr->setPos(*_currentToken);
            }

            TypeParameter* typeParam = _mngr.New<TypeParameter>(vt, ident, kindExpr);
            typeParam->setPos(tpStartPos);
            typeParam->setEndPos(_lastTokenEndPos);

            typeParameters.push_back(typeParam);

        } while (accept(tok::OPER_COMMA) && !accept(tok::TOK_EOF));

        expect(tok::OPER_R_BRACKET, "`" + tok::Operator::OperTypeToString(tok::OPER_R_BRACKET) + "`");
    }

    TypeTuple* typeParamsTuple = _mngr.New<TypeTuple>(typeParameters);
    typeParamsTuple->setPos(startPos);
    typeParamsTuple->setEndPos(_lastTokenEndPos);

    return typeParamsTuple;
}

TypeExpression* Parser::parseTypePrimary(bool allowTypeConstructor) {
    SAVE_POS(startPos)
    std::vector<TypeExpression*> exprs;

    parseAnnotations();

    switch (_currentToken->getTokenType()) {
    case tok::TOK_ID:
        exprs.push_back(parseTypeIdentifier());
        break;

    case tok::TOK_OPER:
        if (accept(tok::OPER_L_BRACKET)) {
            exprs.push_back(parseTypeParameters());
            if (accept(tok::OPER_L_PAREN)) {
                std::vector<TypeExpression*> args;
                parseTuple<TypeTuple, tok::OPER_R_PAREN, TypeExpression*>(args, [&](){return parseTypeExpression();});
                if (expect(tok::OPER_THIN_ARROW, "`->`")) {
                    exprs[0] = createFunctionTypeDecl(static_cast<TypeTuple*>(exprs[0]), args, parseTypeExpression(allowTypeConstructor));
                    // - arrowNecessary will be false since expr.size() == 1,
                    // - accept(tok::OPER_THIN_ARROW) can't happen because it would have been parsed in ret type,
                    // => so we should be good. (ugly though)
                }
            }
        } else if (accept(tok::OPER_L_PAREN)) {
            parseTuple<TypeTuple, tok::OPER_R_PAREN, TypeExpression*>(exprs, [&](){return parseTypeExpression();});
        } else if (accept(tok::OPER_THIN_ARROW)) {
            exprs.push_back(createFunctionTypeDecl(nullptr, {}, parseTypeExpression(allowTypeConstructor)));
        } else {
            reportUnexpectedCurrentToken();
            return nullptr;
        }
        break;

    case tok::TOK_KW: {
        bool isAbstract = accept(tok::KW_ABSTRACT);

        if (accept(tok::KW_CLASS)) {
            return parseClass(isAbstract);
        } else {
            _ctx->reporter().error(*_currentToken, "Unexpected keyword `" + _currentToken->toString() + "`");
            accept();
        }
        break;
    }

    default:
        _ctx->reporter().error(*_currentToken,
                               "Expected identifier | type tuple | class "
                               "; got " + _currentToken->toString());
        accept();
    }

    reportErroneousAnnotations();

    bool arrowNecessary = false;
    TypeExpression* toRet = nullptr;

    if (exprs.size() != 1) {
        arrowNecessary = true; // while waiting for tuples
    } else {
        toRet = exprs[0];
    }

    if ((arrowNecessary && expect(tok::OPER_THIN_ARROW, "`->`")) || accept(tok::OPER_THIN_ARROW)) {
        toRet = createFunctionTypeDecl(nullptr, exprs, parseTypeExpression(allowTypeConstructor));
    }

    if (toRet) {
        toRet->setPos(startPos);
        toRet->setEndPos(_lastTokenEndPos);
    }

    return toRet;
}

KindSpecifyingExpression* Parser::parseKindSpecifyingExpression() {
    KindSpecifyingExpression* toRet = nullptr;
    std::vector<TypeConstructorKindSpecifier::Parameter> exprs;
    bool arrowNecessary = false;

    SAVE_POS(startPos)

    TypeExpression* lowerBound = nullptr;
    TypeExpression* upperBound = nullptr;

    if (!isType(tok::TOK_OPER) ||
            (as<tok::Operator>()->getOpType() != tok::OPER_TIMES &&
             as<tok::Operator>()->getOpType() != tok::OPER_L_BRACKET)) {
        lowerBound = parseTypeExpression(false);
        expect(tok::OPER_LT, "`<`");
    }

    if (accept(tok::OPER_TIMES)) {
        if (accept(tok::OPER_LT)) {
            upperBound = parseTypeExpression(false);
        }

        toRet = _mngr.New<ProperTypeKindSpecifier>(lowerBound, upperBound);
        exprs.push_back(TypeConstructorKindSpecifier::Parameter(common::VAR_T_NONE, toRet));
    } else if (accept(tok::OPER_L_BRACKET)) {
        parseTuple<TypeConstructorKindSpecifier, tok::OPER_R_BRACKET, TypeConstructorKindSpecifier::Parameter>(
                    exprs, [&](){return parseTypeConstructorKindSpecifierParameter();});

        arrowNecessary = true;
    }
    else {
        _ctx->reporter().error(*_currentToken, "Expected proper type or type constructor kind specifier; got `"+ _currentToken->toString() +"`");
        accept();
        return toRet;
    }

    if ((arrowNecessary && expect(tok::OPER_THIN_ARROW, "`->`")) || accept(tok::OPER_THIN_ARROW)) {
        toRet = _mngr.New<TypeConstructorKindSpecifier>(exprs, parseKindSpecifyingExpression());
    } else if (arrowNecessary) {
        return toRet;
    }

    toRet->setPos(startPos);
    toRet->setEndPos(_lastTokenEndPos);
    return toRet;
}

TypeConstructorKindSpecifier::Parameter Parser::parseTypeConstructorKindSpecifierParameter() {
    TypeConstructorKindSpecifier::Parameter param;

    if (accept(tok::KW_IN)) {
        param.varianceType = common::VAR_T_IN;
    } else if (accept(tok::KW_OUT)) {
        param.varianceType = common::VAR_T_OUT;
    } else {
        param.varianceType = common::VAR_T_NONE;
    }

    param.kindExpr = parseKindSpecifyingExpression();

    return param;
}

void Parser::parseAnnotations() {
    SAVE_POS(pos)

    if (!accept(tok::OPER_AT)) {
        return;
    }

    std::string name;
    std::vector<Annotation::ArgumentValue> args;

    if (isType(tok::TOK_ID)) {
        name = as<tok::Identifier>()->toString();
        accept();
    } else if (isType(tok::TOK_KW)) {
        name = as<tok::Keyword>()->toString();
        accept();
    } else {
        _ctx->reporter().error(*_currentToken, "Expected identifier; got " + _currentToken->toString());
        return;
    }

    if (_currentToken->getStartPosition() == _lastTokenEndPos && accept(tok::OPER_L_PAREN)) {
        while (true) {
            if (isType(tok::TOK_BOOL_LIT)) {
                args.push_back(Annotation::ArgumentValue(as<tok::BoolLiteral>()->getValue()));
            } else if (isType(tok::TOK_INT_LIT)) {
                args.push_back(Annotation::ArgumentValue(as<tok::IntLiteral>()->getValue()));
            } else if (isType(tok::TOK_REAL_LIT)) {
                args.push_back(Annotation::ArgumentValue(as<tok::RealLiteral>()->getValue()));
            } else if (isType(tok::TOK_STR_LIT)) {
                args.push_back(Annotation::ArgumentValue(as<tok::StringLiteral>()->getValue()));
            } else {
                _ctx->reporter().error(*_currentToken,
                                       "Expected bool | int | real | string Literal"
                                       "; got " + _currentToken->toString());
                break;
            }

            accept();

            if (accept(tok::OPER_COMMA)) {
                continue;
            } else if (accept(tok::OPER_R_PAREN)) {
                break;
            } else {
                _ctx->reporter().error(*_currentToken,
                                       "Expected `,` or `)`; got " + _currentToken->toString());
                break;
            }
        }
    }

    Annotation* annot = _mngr.New<Annotation>(name, args);
    annot->setPos(pos);
    annot->setEndPos(_lastTokenEndPos);

    _parsedAnnotations.push_back(annot);

    parseAnnotations();
}

std::vector<Annotation*> Parser::consumeAnnotations() {
    std::vector<Annotation*> annots(std::move(_parsedAnnotations));
    _parsedAnnotations.clear();
    return annots;
}

bool Parser::annotationsConsumed() const {
    return _parsedAnnotations.size() == 0;
}

common::Positionnable Parser::annotationsPos() const {
    common::Positionnable pos;
    if (_parsedAnnotations.size() > 0) {
        pos = *_parsedAnnotations[0];
        pos.setEndPos(_parsedAnnotations.back()->getEndPosition());
    }
    return pos;
}

void Parser::reportErroneousAnnotations() {
    if (!annotationsConsumed()) {
        _ctx->reporter().error(annotationsPos(), "Illegal annotation placement");
    }
    _parsedAnnotations.clear();
}

CanUseModules::ModulePath Parser::parseUsing(const common::Positionnable& usingpos, bool asStatement) {
    CanUseModules::ModulePath mpath;

    do {
        if (isType(tok::TOK_ID)) {
            tok::Identifier* id = as<tok::Identifier>();
            mpath.push_back(id->toString());
            accept();
        } else {
            _ctx->reporter().error(*_currentToken, "Expected identifier in module path, but got `" + _currentToken->toString()+ "`");
            break;
        }
    } while (accept(tok::OPER_DOT));

    if (asStatement) {
        expect(tok::OPER_SEMICOLON, "`;`");
    }

    mpath.setPos(usingpos);
    mpath.setEndPos(_lastTokenEndPos);

    return mpath;
}

DefFlags Parser::parseDefFlags() {
    DefFlags toRet = DefFlags::NONE;

    while (isType(tok::TOK_KW)){
        DefFlags flag;

        switch (as<tok::Keyword>()->getKwType()) {
        case tok::KW_EXTERN:    flag = DefFlags::EXTERN; break;
        case tok::KW_ABSTRACT:  flag = DefFlags::ABSTRACT; break;
        case tok::KW_STATIC:    flag = DefFlags::STATIC; break;
        default:                return toRet;
        }

        accept();
        toRet |= flag;
    }

    return toRet;
}

template<typename RETURN_TYPE, tok::OPER_TYPE R_DELIM, typename ELEMENT_TYPE, typename PARSING_FUNC>
RETURN_TYPE* Parser::parseTuple(std::vector<ELEMENT_TYPE>& exprs, const PARSING_FUNC& f) {

    SAVE_POS(startPos)

    if (!accept(R_DELIM)) {
        do {
            if (ELEMENT_TYPE arg = f()){
                exprs.push_back(arg);
            }
        } while (accept(tok::OPER_COMMA) && !accept(tok::TOK_EOF));

        expect(R_DELIM, "`" + tok::Operator::OperTypeToString(R_DELIM) + "`");
    }

    RETURN_TYPE* tuple = _mngr.New<RETURN_TYPE>(exprs);
    tuple->setPos(startPos);
    tuple->setEndPos(_lastTokenEndPos);
    return tuple;
}

This* Parser::makeThis(const common::Positionnable& pos) {
    This* ths = _mngr.New<This>();
    ths->setPos(pos);
    return ths;
}

DefineDecl* Parser::makeFunctionDef(const std::string& name, const std::vector<Expression*>& params, const std::vector<Expression*>& body,
                              const common::Positionnable& pos, DefFlags flags) {

    FunctionCreation* func = _mngr.New<FunctionCreation>(name, nullptr, _mngr.New<Tuple>(params), _mngr.New<Block>(body), nullptr);
    Identifier* defName = _mngr.New<Identifier>(name);
    DefineDecl* defDecl = _mngr.New<DefineDecl>(defName, nullptr, func, flags);

    func->setPos(pos);
    defName->setPos(pos);
    defDecl->setPos(pos);

    return defDecl;
}

Expression* Parser::makeMethodCall(Expression* callee, const std::string& memberName, const std::vector<Expression*>& argExprs,
                                  const common::Positionnable& memberPos, const common::Positionnable& argsPos, TypeTuple* typeArgs) {
    Identifier* id = _mngr.New<Identifier>(memberName);
    Tuple* args = _mngr.New<Tuple>(argExprs);
    id->setPos(memberPos);
    args->setPos(argsPos);

    MemberAccess* mac = _mngr.New<MemberAccess>(callee, id);
    mac->setPos(*callee);
    mac->setEndPos(id->getEndPosition());

    return _mngr.New<FunctionCall>(mac, typeArgs, args);
}

Expression* Parser::makeBinary(Expression* left, Expression* right, tok::Operator* oper) {
    Expression* res;

    switch (oper->getOpType()) {
    case tok::OPER_EQ:
    case tok::OPER_PLUS_EQ:
    case tok::OPER_MINUS_EQ:
    case tok::OPER_TIMES_EQ:
    case tok::OPER_DIV_EQ:
    case tok::OPER_MOD_EQ:
    case tok::OPER_POW_EQ:
    case tok::OPER_B_AND_EQ:
    case tok::OPER_B_OR_EQ:
    case tok::OPER_L_SHIFT_EQ:
    case tok::OPER_R_SHIFT_EQ:

        if (FunctionCall* call = getIfNodeOfType<FunctionCall>(left, _ctx)) {
            std::vector<Expression*> newArgsExprs(call->getArgs());
            newArgsExprs.push_back(right);

            res = makeMethodCall(call->getCallee(), std::string("(") + oper->toString() + ")",
                                 newArgsExprs, *oper, *call->getArgsTuple(), call->getTypeArgsTuple());
        } else {
            std::string baseOper = "";

            switch (oper->getOpType()) {
            case tok::OPER_PLUS_EQ:     baseOper = "+"; break;
            case tok::OPER_MINUS_EQ:    baseOper = "-"; break;
            case tok::OPER_TIMES_EQ:    baseOper = "*"; break;
            case tok::OPER_DIV_EQ:      baseOper = "/"; break;
            case tok::OPER_MOD_EQ:      baseOper = "%"; break;
            case tok::OPER_POW_EQ:      baseOper = "^"; break;
            case tok::OPER_B_AND_EQ:    baseOper = "&"; break;
            case tok::OPER_B_OR_EQ:     baseOper = "|"; break;
            case tok::OPER_L_SHIFT_EQ:  baseOper = "<<"; break;
            case tok::OPER_R_SHIFT_EQ:  baseOper = ">>"; break;
            default: break;
            }

            if (baseOper != "") {
                common::Positionnable pos = *left;
                pos.setEndPos(right->getEndPosition());

                right = makeMethodCall(left, baseOper, {right}, *oper, pos);
                right->setPos(pos);
            }

            res = _mngr.New<AssignmentExpression>(left, right);
        }
        break;
    case tok::OPER_AND: {
        BoolLiteral* fals = _mngr.New<BoolLiteral>(false);
        fals->setPos(*right);
        res = _mngr.New<IfExpression>(left, right, fals, true);
        break;
    }
    case tok::OPER_OR: {
        BoolLiteral* tru = _mngr.New<BoolLiteral>(true);
        tru->setPos(*right);
        res = _mngr.New<IfExpression>(left, tru, right, true);
        break;
    }
    default:
        res = makeMethodCall(left, oper->toString(), {right}, *oper, *right);
        break;
    }

    res->setPos(*left);
    res->setEndPos(_lastTokenEndPos);
    return res;
}

Identifier* Parser::parseOperatorsAsIdentifer() {
    std::string name;
    SAVE_POS(startPos)

    tok::OPER_TYPE op = as<tok::Operator>()->getOpType();

    if (accept(tok::OPER_L_PAREN)) {
        name = "(";
        if (isType(tok::TOK_OPER)) {
            tok::OPER_TYPE inOp = as<tok::Operator>()->getOpType();

            if (inOp >= tok::OPER_EQ && inOp <= tok::OPER_R_SHIFT_EQ) {
                name += as<tok::Operator>()->toString();
                accept();
            } else if (inOp != tok::OPER_R_PAREN) {
                reportUnexpectedCurrentToken();
                accept();
            }
        }
        expect(tok::OPER_R_PAREN, ")");
        name += ")";
    } else if (op == tok::OPER_PLUS    || op == tok::OPER_MINUS   ||
               op == tok::OPER_TIMES   || op == tok::OPER_DIV     ||
               op == tok::OPER_MOD     || op == tok::OPER_POW     ||
               op == tok::OPER_AND     || op == tok::OPER_OR      ||
               op == tok::OPER_BIT_AND || op == tok::OPER_BIT_OR  ||
               op == tok::OPER_L_SHIFT || op == tok::OPER_R_SHIFT ||
               op == tok::OPER_BANG    || op == tok::OPER_TILDE   ||
               op == tok::OPER_EQ_EQ   || op == tok::OPER_LT      ||
               op == tok::OPER_GT      || op == tok::OPER_LE      ||
               op == tok::OPER_GE) {
        name = as<tok::Operator>()->toString();
        accept();
    } else {
        _ctx->reporter().error(*_currentToken,
                               "Expected operator "
                               "; got " + _currentToken->toString());
        accept();
    }

    Identifier* id = _mngr.New<Identifier>(name);
    id->setPos(startPos);
    id->setEndPos(_lastTokenEndPos);
    return id;
}

ClassDecl* Parser::parseClassBody(bool isAbstract, const std::string& className, const common::Positionnable& startPos, bool forAnonymousInstantiation) {
    std::vector<Annotation*> annots(std::move(consumeAnnotations()));
    std::vector<TypeDecl*> tdecls;
    std::vector<TypeSpecifier*> fields;
    std::vector<DefineDecl*> defs;
    std::vector<Expression*> staticExprs;

    TypeExpression* parent = nullptr;
    DefineDecl* trivialConstructor = nullptr;

    if (forAnonymousInstantiation) {
        defs.push_back(makeFunctionDef("new", {}, {}, startPos, DefFlags::CONSTRUCTOR));

        if (!isType(tok::TOK_OPER) || as<tok::Operator>()->getOpType() != tok::OPER_L_BRACE) {
            parent = parseTypeExpression();
        }
        if (!isType(tok::TOK_OPER) || as<tok::Operator>()->getOpType() != tok::OPER_L_BRACE) {
            expect(tok::OPER_L_BRACE, "`{`");
        }
    } else {
        if (accept(tok::OPER_L_PAREN)) {
            trivialConstructor = desugarTrivialConstructor(fields, defs);
            // no need to add manually trivialConstructor to defs, it is
            // already done in desugarTrivialConstructor.
        }

        if (accept(tok::OPER_COLON)) {
            parent = parseTypeExpression();
        }
    }

    if (accept(tok::OPER_L_BRACE)) {
        while (!accept(tok::OPER_R_BRACE) && !accept(tok::TOK_EOF)) {
            parseAnnotations();

            DefFlags flags = parseDefFlags();

            SAVE_POS(externElemPos);

            if (flags % DefFlags::EXTERN && flags % DefFlags::ABSTRACT) {
                _ctx->reporter().error(externElemPos, "`extern` and `abstract` flags are exclusive");
            }
            if (flags % DefFlags::STATIC && flags % DefFlags::ABSTRACT) {
                _ctx->reporter().error(externElemPos, "`static` and `abstract` flags are exclusive");
            }

            if (accept(tok::KW_TPE)) {
                tdecls.push_back(parseTypeDecl(consumeDefFlags(flags, DefFlags::EXTERN)));
            } else if (accept(tok::KW_CLASS)) {
                tdecls.push_back(desugarTopLevelClassDecl(consumeDefFlags(flags, DefFlags::EXTERN | DefFlags::ABSTRACT)));
            } else if (accept(tok::KW_NEW)) {
                Identifier* id = _mngr.New<Identifier>("new");
                id->setPos(externElemPos);
                defs.push_back(parseDef(DefFlags::CONSTRUCTOR | consumeDefFlags(flags, DefFlags::EXTERN), id));
            } else if (accept(tok::KW_DEF)) {
                Identifier* id = isType(tok::TOK_OPER) ? parseOperatorsAsIdentifer() : nullptr;
                defs.push_back(parseDef(consumeDefFlags(flags, DefFlags::EXTERN | DefFlags::ABSTRACT | DefFlags::STATIC), id));
            } else if (accept(tok::KW_REDEF)) {
                Identifier* id = isType(tok::TOK_OPER) ? parseOperatorsAsIdentifer() : nullptr;
                defs.push_back(parseDef(DefFlags::REDEF | consumeDefFlags(flags, DefFlags::EXTERN | DefFlags::ABSTRACT), id));
            } else {
                Identifier* fieldName = parseIdentifier("Expected field name | def");
                expect(tok::OPER_COLON, "`:`");
                TypeExpression* type = parseTypeExpression();

                if (accept(tok::OPER_EQ)) {
                    AssignmentExpression* assignment = _mngr.New<AssignmentExpression>(fieldName, parseExpression());
                    assignment->setPos(*fieldName);
                    assignment->setEndPos(_lastTokenEndPos);
                    staticExprs.push_back(assignment);
                }

                expect(tok::OPER_SEMICOLON, "`;`");

                TypeSpecifier* field = _mngr.New<TypeSpecifier>(fieldName, type);
                field->setPos(*fieldName);
                field->setEndPos(_lastTokenEndPos);

                fields.push_back(field);
            }

            if (flags % DefFlags::EXTERN) {
                _ctx->reporter().error(externElemPos, "Class fields or inner type declarations cannot be declared extern");
            }
            if (flags % DefFlags::ABSTRACT) {
                _ctx->reporter().error(externElemPos, "Class fields or inner type declarations cannot be declared abstract");
            }
            if (flags % DefFlags::STATIC) {
                _ctx->reporter().error(externElemPos, "Class fields, inner type declarations or abstract members cannot be declared static");
            }

            reportErroneousAnnotations();
        }
    }

    // check that the RHS of method definitions are function creations
    // and add static expressions (default initalizations, etc.) in every constructor
    for (DefineDecl* def : defs) {
        if (def->getValue() && !def->isStatic()) {
            if (!isNodeOfType<FunctionCreation>(def->getValue(), _ctx)) {
                _ctx->reporter().error(*def, "A method definition must be a function expression");
            } else if (def->isConstructor() && staticExprs.size() > 0) {
                FunctionCreation* constr = static_cast<FunctionCreation*>(def->getValue());
                Annotable savedAnnot = *constr;
                common::Positionnable savedPos = *constr;

                std::vector<Expression*> newBody(staticExprs);
                newBody.insert(def == trivialConstructor ? newBody.begin() : newBody.end(), constr->getBody());

                *constr = FunctionCreation(constr->getName(), constr->getTypeArgs(), constr->getArgs(), _mngr.New<Block>(newBody), constr->getReturnType());

                *static_cast<Annotable*>(constr) = savedAnnot;
                constr->setPos(savedPos);
            }
        }
    }

    ClassDecl* classDecl = _mngr.New<ClassDecl>(className, parent, tdecls, fields, defs, isAbstract);
    classDecl->setAnnotations(annots);
    classDecl->setPos(startPos);
    classDecl->setEndPos(_lastTokenEndPos);
    return classDecl;
}

DefineDecl* Parser::desugarTrivialConstructor(std::vector<TypeSpecifier*>& fields, std::vector<DefineDecl*>& defs) {
    // desugaring
    SAVE_POS(startPos)

    std::vector<Expression*> params;
    std::vector<Expression*> body;

    if (!accept(tok::OPER_R_PAREN)) {
        do {
            // ADD FIELD
            Identifier* fieldName = parseIdentifier("Expected field name");
            expect(tok::OPER_COLON, "`:`");

            TypeExpression* tp = parseTypeExpression();

            TypeSpecifier* field = _mngr.New<TypeSpecifier>(fieldName, tp);
            field->setPos(*fieldName);
            field->setEndPos(_lastTokenEndPos);

            fields.push_back(field);

            // CREATE PARAM

            Identifier* paramName = _mngr.New<Identifier>(fieldName->getValue() + "$arg");
            paramName->setPos(*fieldName);

            TypeSpecifier* param = _mngr.New<TypeSpecifier>(paramName, tp);
            param->setPos(*field);

            params.push_back(param);

            // CREATE ASSIGNMENT

            body.push_back(_mngr.New<AssignmentExpression>(fieldName, paramName));

        } while (accept(tok::OPER_COMMA) && !accept(tok::TOK_EOF));

        expect(tok::OPER_R_PAREN, "`)`");
    }

    common::Positionnable constrPos = startPos;
    constrPos.setEndPos(_lastTokenEndPos);

    DefineDecl* constr = makeFunctionDef("new", params, body, constrPos, DefFlags::CONSTRUCTOR);
    defs.push_back(constr);

    return constr;
}

TypeDecl* Parser::desugarTopLevelClassDecl(DefFlags flags) {
    TypeIdentifier* typeName = parseTypeIdentifier("Expected type name");
    accept(tok::OPER_EQ);

    TypeExpression* expr;

    if (accept(tok::OPER_L_BRACKET)) {
        TypeTuple* params = parseTypeParameters();
        expr = _mngr.New<TypeConstructorCreation>(typeName->getValue(), params,
                                                  parseClassBody(flags % DefFlags::ABSTRACT, typeName->getValue(), *typeName));
        expr->setPos(*params);
        expr->setEndPos(_lastTokenEndPos);
    } else {
        expr = parseClassBody(flags % DefFlags::ABSTRACT, typeName->getValue(), *typeName);
    }

    TypeDecl* typeDecl = _mngr.New<TypeDecl>(typeName, expr, flags % DefFlags::EXTERN);
    typeDecl->setPos(*typeName);
    typeDecl->setEndPos(_lastTokenEndPos);
    return typeDecl;
}

Expression* Parser::parseNew(const common::Positionnable& newPos) {
    SAVE_POS(classPos)

    Instantiation* inst = _mngr.New<Instantiation>(parseClassBody(false, AnonymousClassName, classPos, true));
    FunctionCall* call = _mngr.New<FunctionCall>(inst, nullptr, _mngr.New<Tuple>(std::vector<Expression*>()));

    inst->setPos(newPos);
    inst->setEndPos(_lastTokenEndPos);
    call->setPos(*inst);

    return call;
}

}
