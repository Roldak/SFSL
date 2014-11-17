//
//  ASTVisitor.cpp
//  SFSL
//
//  Created by Romain Beguet on 16.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "ASTVisitor.h"
#include "../../Common/CompilationContext.h"

namespace sfsl {

namespace ast {

void ASTVisitor::visit(ASTNode *node) {
    throw common::CompilationFatalError("unimplemented visitor");
}



}

}
