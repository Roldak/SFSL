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
    ScopePossessorVisitor(std::shared_ptr<common::CompilationContext>& ctx);

    template<typename T, typename U>
    T* createSymbol(U* node);

    void tryAddSymbol(sym::Symbol* sym);

    sym::Scope* _curScope;
};

/**
 * @brief
 */
class ScopeGeneration : public ScopePossessorVisitor {
public:

    ScopeGeneration(std::shared_ptr<common::CompilationContext>& ctx);

    virtual void visit(Program* prog);

    virtual void visit(ModuleDecl* module);
    virtual void visit(ClassDecl* clss);
    virtual void visit(DefineDecl* decl);

    virtual void visit(Block* block);
    virtual void visit(FunctionCreation* func);

private:

    void pushScope(sym::Scoped* scoped = nullptr, bool isDefScope = false);
    void popScope();
};

/**
 * @brief
 */
class SymbolAssignation : public ScopePossessorVisitor {
public:

    SymbolAssignation(std::shared_ptr<common::CompilationContext>& ctx);

    virtual void visit(ModuleDecl* mod);
    virtual void visit(ClassDecl* clss);
    virtual void visit(DefineDecl* decl);

    virtual void visit(BinaryExpression* exp);
    virtual void visit(MemberAccess* mac);
    virtual void visit(Block* block);
    virtual void visit(FunctionCreation* func);

    virtual void visit(Identifier* id);

private:
};

}

}

#endif
