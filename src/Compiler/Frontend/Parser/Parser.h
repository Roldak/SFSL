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
#include <functional>

#include "../Lexer/Lexer.h"
#include "../Lexer/Tokens.h"

#include "../../Common/CompilationContext.h"
#include "../../../Common/AbstractPrimitiveNamer.h"

#include "../AST/Nodes/Program.h"
#include "../AST/Nodes/Expressions.h"
#include "../AST/Nodes/TypeExpressions.h"
#include "../AST/Nodes/KindExpressions.h"

#include "../AST/Utils/Annotations.h"

namespace sfsl {

namespace ast {

/**
 * @brief The Parser is used to transform a sequence of tokens into an Abstract Syntax Tree
 */
class Parser final {
public:

    /**
     * @brief Creates a Parser object
     * @param ctx The compilation context used throughout the parsing to report errors and allocate memory
     * @param lexer The lexer from which to fetch the tokens during the parsing
     */
    Parser(CompCtx_Ptr& ctx, lex::Lexer& lexer, const common::AbstractPrimitiveNamer* namer);

    /**
     * @brief Start the parsing process
     */
    ast::Program* parse();

    /**
     * @brief Parses a single expression
     */
    ast::Expression* parseSingleExpression();

    /**
     * @return Start parsing the given input assuming it is a type expression
     */
    ast::TypeExpression* parseType();

private:

    // Static Members

    static const std::string AnonymousClassName;
    static const std::string AnonymousTypeConstructorName;
    static const std::string AnonymousFunctionName;

    // Utils

    bool isType(tok::TOK_TYPE type);

    bool accept(tok::TOK_TYPE type);
    bool accept(tok::OPER_TYPE type);
    bool accept(tok::KW_TYPE type);
    void accept();

    template<typename T>
    bool expect(T type, const std::string& expected, bool fatal = false);

    template<typename T>
    T expectSemicolonAndReturn(T expr);

    template<typename T>
    T* as();

    void reportUnexpectedCurrentToken();

    // Parsing

    template<typename T>
    T* parseIdentifierHelper(const std::string& errMsg);

    ast::Identifier* parseIdentifier(const std::string& errMsg = "");
    ast::TypeIdentifier* parseTypeIdentifier(const std::string& errMsg = "");

    ast::Program* parseProgram();
    ast::ModuleDecl* parseModule();
    ast::DefineDecl* parseDef(DefFlags flags, ast::Identifier* name = nullptr);
    ast::ClassDecl* parseClass(bool isAbstract);
    ast::TypeDecl* parseTypeDecl(DefFlags flags);

        // statements

    ast::Expression* parseStatement();

        // expressions

    ast::Expression* parseExpression();
    ast::Expression* parseUnary();
    ast::Expression* parseBinary(ast::Expression* left, int precedence);
    ast::Expression* parsePrimary();
    ast::TypeSpecifier* parseTypeSpecifier(ast::Identifier* id);

    ast::Block* parseBlock();
    ast::IfExpression* parseIf(bool asStatement);

    ast::Expression* parseSpecialBinaryContinuity(ast::Expression* left);
    ast::Tuple* parseTuple();

        // type expression

    ast::TypeExpression* parseTypeExpression(bool allowTypeConstructor = true);
    ast::TypeExpression* parseTypeBinary(ast::TypeExpression* left, int precedence, bool allowTypeConstructor);
    ast::TypeExpression* parseTypePrimary(bool allowTypeConstructor = true);
    ast::TypeExpression* createFunctionTypeDecl(const TypeTuple* typeParams, const std::vector<TypeExpression*>& args, TypeExpression* ret);
    ast::TypeTuple* parseTypeTuple();

    ast::TypeTuple* parseTypeParameters();

        // kind expression

    ast::KindSpecifyingExpression* parseKindSpecifyingExpression();
    ast::TypeConstructorKindSpecifier::Parameter parseTypeConstructorKindSpecifierParameter();

        // annotations

    void parseAnnotations();
    std::vector<Annotation*> consumeAnnotations();
    bool annotationsConsumed() const;
    common::Positionnable annotationsPos() const;
    void reportErroneousAnnotations();

        // others

    ast::CanUseModules::ModulePath parseUsing(const common::Positionnable& usingpos, bool asStatement);

    template<typename RETURN_TYPE, tok::OPER_TYPE R_DELIM, typename ELEMENT_TYPE, typename PARSING_FUNC>
    RETURN_TYPE* parseTuple(std::vector<ELEMENT_TYPE>& exprs, const PARSING_FUNC& f);

    DefFlags parseDefFlags();

    ast::This* makeThis(const common::Positionnable& pos);

    ast::DefineDecl* makeFunctionDef(const std::string& name, const std::vector<Expression*>& params, const std::vector<Expression*>& body,
                                const common::Positionnable& pos, DefFlags flags);

    ast::Expression* makeMethodCall(Expression* callee, const std::string& memberName, const std::vector<Expression*>& argExprs,
                                    const common::Positionnable& memberPos, const common::Positionnable& argsPos, TypeTuple* typeArgs = nullptr);

    ast::Expression* makeBinary(Expression* left, Expression* right, tok::Operator* oper);
    ast::Identifier* parseOperatorsAsIdentifer();

    ast::ClassDecl* parseClassBody(bool isAbstract, const std::string& className,
                                   const common::Positionnable& startPos, bool forAnonymousInstantiation = false);

    DefineDecl* desugarTrivialConstructor(std::vector<TypeSpecifier*>& fields, std::vector<DefineDecl*>& defs);
    TypeDecl* desugarTopLevelClassDecl(DefFlags flags);
    Expression* parseNew(const common::Positionnable& newPos);

    // Members

    std::vector<Annotation*> _parsedAnnotations;

    CompCtx_Ptr _ctx;
    common::AbstractMemoryManager& _mngr;
    lex::Lexer& _lex;
    const common::AbstractPrimitiveNamer* _namer;

    size_t _lastTokenEndPos;
    tok::Token* _currentToken;

    std::string _currentTypeName;
    std::string _currentDefName;
};

}

}

#endif
