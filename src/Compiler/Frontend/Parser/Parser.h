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
#include "../Lexer/Tokens/Keyword.h"
#include "../Lexer/Tokens/Operators.h"
#include "../Lexer/Tokens/Identifier.h"
#include "../../Common/CompilationContext.h"

#include "../AST/Nodes/Program.h"
#include "../AST/Nodes/Expressions.h"
#include "../AST/Nodes/TypeExpressions.h"
#include "../AST/Nodes/KindExpressions.h"

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
    Parser(CompCtx_Ptr& ctx, lex::Lexer& lexer);

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
    T* as();

    // Parsing

    template<typename T>
    T* parseIdentifierHelper(const std::string errMsg);

    ast::Identifier* parseIdentifier(const std::string& errMsg = "");
    ast::TypeIdentifier* parseTypeIdentifier(const std::string& errMsg = "");

    ast::Program* parseProgram();
    ast::ModuleDecl* parseModule();
    ast::DefineDecl* parseDef(bool asStatement, bool isRedef, bool isExtern);
    ast::ClassDecl* parseClass();
    ast::TypeDecl* parseType(bool asStatement);

        // statements

    ast::Expression* parseStatement();

        // expressions

    ast::Expression* parseExpression();
    ast::Expression* parseBinary(ast::Expression* left, int precedence);
    ast::Expression* parsePrimary();
    ast::TypeSpecifier* parseTypeSpecifier(ast::Identifier* id);

    ast::TypeExpression* parseTypeExpression(bool allowTypeConstructor = true);
    ast::TypeExpression* parseTypeBinary(ast::TypeExpression* left, int precedence, bool allowTypeConstructor);
    ast::TypeExpression* parseTypePrimary();
    ast::KindSpecifier* parseKindSpecifier(ast::TypeIdentifier* id);

    ast::KindSpecifyingExpression* parseKindSpecifyingExpression();

    ast::Block* parseBlock();
    ast::IfExpression* parseIf(bool asStatement);
    ast::This* parseThis(const common::Positionnable& pos);

    ast::Expression* parseSpecialBinaryContinuity(ast::Expression* left);
    ast::Tuple* parseTuple();
    ast::TypeTuple* parseTypeTuple();

    ast::CanUseModules::ModulePath parseUsing(const common::Positionnable& usingpos, bool asStatement);

        // others

    template<typename RETURN_TYPE, tok::OPER_TYPE R_DELIM, typename ELEMENT_TYPE, typename PARSING_FUNC>
    RETURN_TYPE* parseTuple(std::vector<ELEMENT_TYPE*>& exprs, const PARSING_FUNC& f);

    ast::Expression* makeBinary(Expression* left, Expression* right, tok::Operator* oper);

    // Members

    CompCtx_Ptr _ctx;
    common::AbstractMemoryManager& _mngr;
    lex::Lexer& _lex;

    size_t _lastTokenEndPos;
    tok::Token* _currentToken;

    std::string _currentTypeName;
    std::string _currentDefName;
};

}

}

#endif
