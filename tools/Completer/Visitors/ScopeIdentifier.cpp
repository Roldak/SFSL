//
//  ScopeIdentifer.cpp
//  SFSL
//
//  Created by Romain Beguet on 20.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "ScopeIdentifier.h"

namespace sfsl {

namespace complete {

ScopeIdentifer::ScopeIdentifer(CompCtx_Ptr& ctx) : ASTImplicitVisitor(ctx) {

}

ScopeIdentifer::~ScopeIdentifer() {

}

void ScopeIdentifer::visit(ast::Program* prog) {

}

}

}
