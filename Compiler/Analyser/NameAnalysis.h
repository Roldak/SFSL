//
//  NameAnalysis.h
//  SFSL
//
//  Created by Romain Beguet on 13.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__NameAnalysis__
#define __SFSL__NameAnalysis__

#include <iostream>
#include "../AST/Visitors/ASTVisitor.h"

namespace sfsl {

namespace ast {

/**
 * @brief
 */
class ScopePossessorVisitor : public ASTVisitor {
protected:
    ScopePossessorVisitor(CompCtx_Ptr& ctx);
    virtual ~ScopePossessorVisitor();

    template<typename T, typename U>
    T* createSymbol(U* node);

    sym::DefinitionSymbol* createSymbol(DefineDecl* node);
    sym::TypeSymbol* createSymbol(TypeDecl* node);

    void tryAddSymbol(sym::Symbol* sym);

    sym::Scope* _curScope;
};

/**
 * @brief
 */
class ScopeGeneration : public ScopePossessorVisitor {
public:

    ScopeGeneration(CompCtx_Ptr& ctx);
    virtual ~ScopeGeneration();

    virtual void visit(Program* prog) override;

    virtual void visit(ModuleDecl* module) override;
    virtual void visit(TypeDecl* tdecl) override;
    virtual void visit(ClassDecl* clss) override;
    virtual void visit(DefineDecl* decl) override;

    virtual void visit(TypeConstructorCreation* typeconstructor) override;

    virtual void visit(Block* block) override;
    virtual void visit(FunctionCreation* func) override;

private:

    void pushScope(sym::Scoped* scoped = nullptr, bool isDefScope = false);
    void popScope();
};

/**
 * @brief
 */
class SymbolAssignation : public ScopePossessorVisitor {
public:

    SymbolAssignation(CompCtx_Ptr& ctx);
    virtual ~SymbolAssignation();

    virtual void visit(ModuleDecl* mod) override;
    virtual void visit(ClassDecl* clss) override;
    virtual void visit(DefineDecl* decl) override;

    virtual void visit(TypeConstructorCreation* typeconstructor) override;

    virtual void visit(BinaryExpression* exp) override;
    virtual void visit(MemberAccess* mac) override;
    virtual void visit(Block* block) override;
    virtual void visit(FunctionCreation* func) override;
    virtual void visit(TypeSpecifier* tps) override;
    virtual void visit(Identifier* id) override;

private:

    void createVar(Identifier* id);
    void createObjectType(Identifier* id);
    void createTypeConstructor(TypeIdentifier* id, TypeTuple* ttuple);

    template<typename T, typename S>
    void initCreated(T* id, S* s);

    void assignFromStaticScope(MemberAccess* mac, sym::Scoped* scoped, const std::string& typeName);
    void assignFromTypeSymbol(MemberAccess* mac, sym::TypeSymbol* tsym);

};

}

}

#endif
