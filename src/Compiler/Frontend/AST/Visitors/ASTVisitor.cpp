//
//  ASTVisitor.cpp
//  SFSL
//
//  Created by Romain Beguet on 16.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "ASTImplicitVisitor.h"
#include "../../../Common/CompilationContext.h"

namespace sfsl {

namespace ast {

ASTVisitor::ASTVisitor(CompCtx_Ptr &ctx) : _ctx(ctx), _mngr(ctx.get()->memoryManager()) {

}

ASTVisitor::~ASTVisitor() {

}

}

}
