//
//  CapturesChecker.h
//  SFSL
//
//  Created by Romain Beguet on 11.11.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__CapturesChecker__
#define __SFSL__CapturesChecker__

#include "Compiler/Frontend/AST/Visitors/ASTImplicitVisitor.h"

namespace sfsl {

namespace test {

class CapturesChecker : public ast::ASTImplicitVisitor {
public:
    CapturesChecker(CompCtx_Ptr& ctx);
    virtual ~CapturesChecker();

    virtual void visit(ast::ClassDecl* clss) override;
    virtual void visit(ast::FunctionCreation* func) override;

private:

    void visitClosure(const common::Positionnable& pos, ast::Annotable* annotable,
                      common::HasManageableUserdata* closureData);
};

}

}

#endif
