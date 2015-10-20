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

namespace sfsl {

namespace complete {

class ScopeIdentifer : public ast::ASTImplicitVisitor {
public:
    ScopeIdentifer(CompCtx_Ptr& ctx);
    virtual ~ScopeIdentifer();

    virtual void visit(ast::Program* prog) override;

private:
};

}

}

#endif
