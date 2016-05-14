//
//  Parser.cpp
//  SFSL
//
//  Created by Romain Beguet on 16.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Parser.h"

#include "../Lexer/Tokens.h"
#include "../AST/Visitors/ASTTypeIdentifier.h"

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

bool consumeBool(bool& val) {
    bool tmp = val;
    val = false;
    return tmp;
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

    while (!accept(tok::OPER_R_BRACE)) {
        parseAnnotations();

        SAVE_POS(keywordPos)
        bool isExtern = accept(tok::KW_EXTERN);
        SAVE_POS(externElemPos)

        if (accept(tok::KW_MODULE)) {
            mods.push_back(parseModule());
        } else if (accept(tok::KW_TPE)) {
            types.push_back(parseType(false));
        } else if (accept(tok::KW_DEF)) {
            decls.push_back(parseDef(false, false, consumeBool(isExtern), false));
        } else if (accept(tok::KW_USING)) {
            usings.push_back(parseUsing(keywordPos, false));
        } else {
            expect(tok::OPER_R_BRACE, "`}`");
            break;
        }

        if (isExtern) {
            _ctx->reporter().error(externElemPos, "Modules or type declarations cannot be declared extern");
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

DefineDecl* Parser::parseDef(bool asStatement, bool isRedef, bool isExtern, bool isAbstract, Identifier* name) {
    std::vector<Annotation*> annots(std::move(consumeAnnotations()));

    Identifier* defName = (name ? name : parseIdentifier("Expected definition name"));

    TypeExpression* typeSpecifier = nullptr;
    Expression* expr = nullptr;

    if (isExtern || isAbstract) {
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

    if (asStatement) {
        expect(tok::OPER_SEMICOLON, "`;`");
    }

    DefineDecl* defDecl = _mngr.New<DefineDecl>(defName, typeSpecifier, expr, isRedef, isExtern, isAbstract);
    defDecl->setAnnotations(annots);
    defDecl->setPos(*defName);
    defDecl->setEndPos(_lastTokenEndPos);
    return defDecl;
}

ClassDecl* Parser::parseClass(bool isAbstractClass) {
    SAVE_POS(startPos)

    std::vector<Annotation*> annots(std::move(consumeAnnotations()));

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

    std::vector<TypeDecl*> tdecls;
    std::vector<TypeSpecifier*> fields;
    std::vector<DefineDecl*> defs;

    while (!accept(tok::OPER_R_BRACE) && !accept(tok::TOK_EOF)) {
        parseAnnotations();

        bool isExtern = accept(tok::KW_EXTERN);
        bool isAbstract = accept(tok::KW_ABSTRACT);

        SAVE_POS(externElemPos);

        if (isExtern && isAbstract) {
            _ctx->reporter().error(externElemPos, "`extern` and `abstract` flags are exclusive");
        }

        if (accept(tok::KW_TPE)) {
            tdecls.push_back(parseType(false));
        } else if (accept(tok::KW_NEW)) {
            Identifier* id = _mngr.New<Identifier>("new");
            id->setPos(externElemPos);
            defs.push_back(parseDef(false, false, consumeBool(isExtern), false, id));
        } else if (accept(tok::KW_DEF)) {
            Identifier* id = isType(tok::TOK_OPER) ? parseOperatorsAsIdentifer() : nullptr;
            defs.push_back(parseDef(false, false, consumeBool(isExtern), consumeBool(isAbstract), id));
        } else if (accept(tok::KW_REDEF)) {
            Identifier* id = isType(tok::TOK_OPER) ? parseOperatorsAsIdentifer() : nullptr;
            defs.push_back(parseDef(false, true, consumeBool(isExtern), consumeBool(isAbstract), id));
        } else {
            Identifier* fieldName = parseIdentifier("Expected field name | def");
            expect(tok::OPER_COLON, "`:`");
            TypeExpression* type = parseTypeExpression();
            expect(tok::OPER_SEMICOLON, "`;`");

            TypeSpecifier* field = _mngr.New<TypeSpecifier>(fieldName, type);
            field->setPos(*fieldName);
            field->setEndPos(_lastTokenEndPos);

            fields.push_back(field);
        }

        if (isExtern) {
            _ctx->reporter().error(externElemPos, "Class fields or inner type declarations cannot be declared extern");
        }
        if (isAbstract) {
            _ctx->reporter().error(externElemPos, "Class fields or inner type declarations cannot be declared abstract");
        }

        reportErroneousAnnotations();
    }

    ClassDecl* classDecl = _mngr.New<ClassDecl>(className, parent, tdecls, fields, defs, isAbstractClass);
    classDecl->setAnnotations(annots);
    classDecl->setPos(startPos);
    classDecl->setEndPos(_lastTokenEndPos);
    return classDecl;
}

TypeDecl* Parser::parseType(bool asStatement) {
    std::vector<Annotation*> annots(std::move(consumeAnnotations()));

    TypeIdentifier* typeName = parseTypeIdentifier("Expected type name");
    accept(tok::OPER_EQ);

    std::string lastTypeName = _currentTypeName;
    _currentTypeName = typeName->getValue();

    TypeExpression* expr = parseTypeExpression();

    _currentTypeName = lastTypeName;

    if (asStatement) {
        expect(tok::OPER_SEMICOLON, "`;`");
    }

    TypeDecl* typeDecl = _mngr.New<TypeDecl>(typeName, expr);
    typeDecl->setAnnotations(annots);
    typeDecl->setPos(*typeName);
    typeDecl->setEndPos(_lastTokenEndPos);
    return typeDecl;
}

Expression* Parser::parseStatement() {
    SAVE_POS(startPos)

    parseAnnotations();

    if (isType(tok::TOK_KW) && as<tok::Keyword>()->getKwType() != tok::KW_THIS) {
        tok::KW_TYPE kw = as<tok::Keyword>()->getKwType();
        accept();

        switch (kw) {
        case tok::KW_DEF:   return parseDef(true, false, false, false);
        case tok::KW_TPE:   return parseType(true);
        case tok::KW_IF:
            reportErroneousAnnotations();
            return parseIf(true);

        case tok::KW_REDEF:
        case tok::KW_ABSTRACT:
            _ctx->reporter().error(startPos, "`" + tok::Keyword::KeywordTypeToString(kw) +
                                   "` keyword can only be used inside a class scope");
            reportErroneousAnnotations();
            return nullptr;
        default:
            _ctx->reporter().error(startPos, "Unexpected keyword `" + tok::Keyword::KeywordTypeToString(kw) + "`");
            reportErroneousAnnotations();
            return nullptr;
        }
    } else if (accept(tok::OPER_L_BRACE)) {
        reportErroneousAnnotations();
        return parseBlock();
    } else {
        reportErroneousAnnotations();
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
                Expression* right = parsePrimary();

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
    SAVE_POS(startPos)
    Expression* toRet = nullptr;
    bool shouldReportAnnotations = true;

    parseAnnotations();

    switch (_currentToken->getTokenType()) {
    case tok::TOK_BOOL_LIT:
        toRet = _mngr.New<BoolLitteral>(as<tok::BoolLitteral>()->getValue());
        toRet->setPos(*_currentToken);
        accept();
        break;

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
        toRet = _mngr.New<StringLitteral>(as<tok::StringLitteral>()->getValue());
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
            Tuple* tuple = static_cast<Tuple*>(toRet = parseTuple());

            if (tuple->getExpressions().size() == 1) {
                toRet = tuple->getExpressions()[0];
            }

            shouldReportAnnotations = false;
        }
        else if (accept(tok::OPER_L_BRACKET)) {
            SAVE_POS(pos)

            std::vector<Annotation*> annots(std::move(consumeAnnotations()));

            TypeTuple* typeParams = parseTypeParameters(false);

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
            toRet->setPos(pos);
            toRet->setEndPos(_lastTokenEndPos);
        }
        else if (accept(tok::OPER_L_BRACE)) {
            toRet = parseBlock();
        } else {
            _ctx->reporter().error(*_currentToken, "Unexpected token `"+ _currentToken->toString() +"`");
            accept();
        }
        break;

    case tok::TOK_KW:
        if (accept(tok::KW_IF)) {
            toRet = parseIf(false);
        } else if (accept(tok::KW_THIS)) {
            toRet = parseThis(startPos);
        } else {
            _ctx->reporter().error(*_currentToken, "Unexpected keyword `" + _currentToken->toString() + "`");
            accept();
        }
        break;

    default:
        _ctx->reporter().error(*_currentToken,
                               "Expected int litteral | real litteral | string litteral "
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

This* Parser::parseThis(const common::Positionnable& pos) {
    This* ths = _mngr.New<This>();
    ths->setPos(pos);
    return ths;
}

Expression* Parser::parseSpecialBinaryContinuity(Expression* left) {
    Expression* res = nullptr;

    if (accept(tok::OPER_L_PAREN)) {
        res = _mngr.New<FunctionCall>(left, nullptr, parseTuple());
    } else if (accept(tok::OPER_L_BRACKET)) {
        TypeTuple* typeArgs = parseTypeTuple();
        expect(tok::OPER_L_PAREN, "`(`");
        res = _mngr.New<FunctionCall>(left, typeArgs, parseTuple());
    } else if (accept(tok::OPER_FAT_ARROW)) {
        std::vector<Annotation*> annots(std::move(consumeAnnotations()));
        res = _mngr.New<FunctionCreation>(
                    _currentDefName.empty() ? AnonymousFunctionName : _currentDefName,
                    nullptr, left, parseExpression());
        static_cast<FunctionCreation*>(res)->setAnnotations(annots);
    } else if (accept(tok::OPER_THIN_ARROW)) {
        std::vector<Annotation*> annots(std::move(consumeAnnotations()));
        TypeExpression* retType = parseTypeExpression(false);
        expect(tok::OPER_FAT_ARROW, "`=>`");
        res = _mngr.New<FunctionCreation>(
                    _currentDefName.empty() ? AnonymousFunctionName : _currentDefName,
                    nullptr, left, parseExpression(), retType);
        static_cast<FunctionCreation*>(res)->setAnnotations(annots);
    } else if (accept(tok::OPER_DOT)) {
        Identifier* id = isType(tok::TOK_OPER) ? parseOperatorsAsIdentifer() : parseIdentifier("Expected attribute / method name");
        res = _mngr.New<MemberAccess>(left, id);
    } // no match is not an error

    if (res) {
        res->setPos(*left);
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
    while (isType(tok::TOK_OPER)) {
        tok::Operator* op = as<tok::Operator>();
        int newOpPrec = op->getPrecedence();

        if (newOpPrec >= precedence && op->getOpType() != tok::OPER_EQ) {

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

TypeTuple* Parser::parseTypeParameters(bool allowVarianceAnnotations) {
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

            if (vt != common::VAR_T_NONE && !allowVarianceAnnotations) {
                _ctx->reporter().error(tpStartPos, "Variance annotation are not allowed in this context");
            }

            ident = parseTypeIdentifier("Expected type name");

            if (accept(tok::OPER_COLON)) {
                kindExpr = parseKindSpecifyingExpression();
            } else {
                kindExpr = _mngr.New<ProperTypeKindSpecifier>();
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
            exprs.push_back(parseTypeParameters(true));
            if (accept(tok::OPER_L_PAREN)) {
                std::vector<TypeExpression*> args;
                parseTuple<TypeTuple, tok::OPER_R_PAREN, TypeExpression*>(args, [&](){return parseTypeExpression();});
                if (expect(tok::OPER_THIN_ARROW, "`->`")) {
                    exprs[0] = createFunctionTypeDecl(static_cast<TypeTuple*>(exprs[0]), args, parseTypeExpression(allowTypeConstructor));
                    // - arrowNecessary will be false since expr.size() == 1,
                    // - accept(tok::OPER_THIN_ARROW) can't happen because it would have been parsed in ret type,
                    // => so we're good normally. (ugly though)
                }
            }
        } else if (accept(tok::OPER_L_PAREN)) {
            parseTuple<TypeTuple, tok::OPER_R_PAREN, TypeExpression*>(exprs, [&](){return parseTypeExpression();});
        } else {
            _ctx->reporter().error(*_currentToken, "Unexpected token `"+ _currentToken->toString() +"`");
            accept();
        }
        break;

    case tok::TOK_KW:
        if (accept(tok::KW_ABSTRACT)) {
            expect(tok::KW_CLASS, "`class`");
            return parseClass(true);
        } if (accept(tok::KW_CLASS)) {
            return parseClass(false);
        } else {
            _ctx->reporter().error(*_currentToken, "Unexpected keyword `" + _currentToken->toString() + "`");
            accept();
        }
        break;

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

    switch (_currentToken->getTokenType()) {
    case tok::TOK_OPER:
        if (accept(tok::OPER_TIMES)) {
            toRet = _mngr.New<ProperTypeKindSpecifier>();
            exprs.push_back(TypeConstructorKindSpecifier::Parameter(common::VAR_T_NONE, toRet));
        } else if (accept(tok::OPER_L_BRACKET)) {
            parseTuple<TypeConstructorKindSpecifier, tok::OPER_R_BRACKET, TypeConstructorKindSpecifier::Parameter>(
                        exprs, [&](){return parseTypeConstructorKindSpecifierParameter();});

            arrowNecessary = true;
        }
        else {
            _ctx->reporter().error(*_currentToken, "Unexpected token `"+ _currentToken->toString() +"`");
            accept();
            break;
        }

        if ((arrowNecessary && expect(tok::OPER_THIN_ARROW, "`->`")) || accept(tok::OPER_THIN_ARROW)) {
            toRet = _mngr.New<TypeConstructorKindSpecifier>(exprs, parseKindSpecifyingExpression());
        }

        toRet->setPos(startPos);
        toRet->setEndPos(_lastTokenEndPos);
        break;

    default:
        _ctx->reporter().error(*_currentToken,
                               "Expected identifier | type tuple | class "
                               "; got " + _currentToken->toString());
        accept();
    }

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
    } else {
        _ctx->reporter().error(*_currentToken, "Expected identifier; got " + _currentToken->toString());
        return;
    }

    if (_currentToken->getStartPosition() == _lastTokenEndPos && accept(tok::OPER_L_PAREN)) {
        while (true) {
            if (isType(tok::TOK_BOOL_LIT)) {
                args.push_back(Annotation::ArgumentValue(as<tok::BoolLitteral>()->getValue()));
            } else if (isType(tok::TOK_INT_LIT)) {
                args.push_back(Annotation::ArgumentValue(as<tok::IntLitteral>()->getValue()));
            } else if (isType(tok::TOK_REAL_LIT)) {
                args.push_back(Annotation::ArgumentValue(as<tok::RealLitteral>()->getValue()));
            } else if (isType(tok::TOK_STR_LIT)) {
                args.push_back(Annotation::ArgumentValue(as<tok::StringLitteral>()->getValue()));
            } else {
                _ctx->reporter().error(*_currentToken,
                                       "Expected bool | int | real | string litteral"
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

Expression* Parser::makeBinary(Expression* left, Expression* right, tok::Operator* oper) {
    Expression* res;

    switch (oper->getOpType()) {
    case tok::OPER_EQ:
        res = _mngr.New<AssignmentExpression>(left, right);
        break;
    default:
        Identifier* id = _mngr.New<Identifier>(oper->toString());
        Tuple* args = _mngr.New<Tuple>(std::vector<Expression*>{right});
        id->setPos(*oper);
        args->setPos(*right);
        res = _mngr.New<MemberAccess>(left, id);
        res = _mngr.New<FunctionCall>(res, nullptr, args);
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
        expect(tok::OPER_R_PAREN, ")");
        name = "()";
    } else if (op == tok::OPER_PLUS   || op == tok::OPER_MINUS ||
               op == tok::OPER_TIMES  || op == tok::OPER_DIV   ||
               op == tok::OPER_MOD    || op == tok::OPER_POW   ||
               op == tok::OPER_AND    || op == tok::OPER_OR    ||
               op == tok::OPER_EQ_EQ  || op == tok::OPER_LT    ||
               op == tok::OPER_GT     || op == tok::OPER_LE    ||
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

}
