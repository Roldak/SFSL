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

class ScopeIdentifer : public ast::ASTImplicitVisitor {
public:
    ScopeIdentifer(CompCtx_Ptr& ctx, AbstractScopeReporter* rep);
    virtual ~ScopeIdentifer();

    virtual void visit(ast::ModuleDecl* module) override;
    virtual void visit(ast::TypeDecl* tdecl) override;
    virtual void visit(ast::ClassDecl* clss) override;
    virtual void visit(ast::DefineDecl* decl) override;

    virtual void visit(ast::TypeConstructorCreation* tc) override;

    virtual void visit(ast::Block* block) override;
    virtual void visit(ast::FunctionCreation* func) override;

private:

    void report(common::Positionnable* pos);
    void saveAndReport(common::Positionnable* pos);
    void save();
    void restore();

    AbstractScopeReporter* _rep;

    size_t _curId;
    std::vector<size_t> _curScopePath;
};

}

}

#endif
