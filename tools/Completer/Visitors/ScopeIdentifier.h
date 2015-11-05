//
//  ScopeIdentifer.h
//  SFSL
//
//  Created by Romain Beguet on 20.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__ScopeIdentifer__
#define __SFSL__ScopeIdentifer__

#include "Compiler/Frontend/AST/Visitors/ASTImplicitVisitor.h"
#include "../ScopeReporter.h"

namespace sfsl {

namespace complete {

class ScopeWalker : public ast::ASTImplicitVisitor {
public:
    ScopeWalker(CompCtx_Ptr& ctx);
    virtual ~ScopeWalker();

    virtual void visit(ast::ModuleDecl* module) override;
    virtual void visit(ast::TypeDecl* tdecl) override;
    virtual void visit(ast::ClassDecl* clss) override;
    virtual void visit(ast::DefineDecl* decl) override;

    virtual void visit(ast::TypeConstructorCreation* tc) override;

    virtual void visit(ast::Block* block) override;
    virtual void visit(ast::FunctionCreation* func) override;

protected:
    virtual void enterScope(ast::ASTNode* scope);
    virtual void leaveScope();

    size_t _curId;
    std::vector<size_t> _curScopePath;
};

class ScopeReporter : public ScopeWalker {
public:
    ScopeReporter(CompCtx_Ptr& ctx, AbstractScopeReporter* rep);
    virtual ~ScopeReporter();

private:
    virtual void enterScope(ast::ASTNode*) override;
    virtual void leaveScope() override;

    void report(common::Positionnable* pos);

    AbstractScopeReporter* _rep;

};

class ScopeFinder : public ScopeWalker {
public:
    ScopeFinder(CompCtx_Ptr& ctx, const std::vector<size_t>& scopePath);
    virtual ~ScopeFinder();

    virtual void visit(ast::DefineDecl* decl) override;

    virtual void visit(ast::Block* block) override;
    virtual void visit(ast::FunctionCreation* func) override;

    ast::Block* getBlock();

private:

    bool isCurrentScopePrefixOfDest();

    std::vector<size_t> _scopePath;
    ast::Block* _block;
};

}

}

#endif
