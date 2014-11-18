//
//  ASTVisitorTemplate.cpp
//  SFSL
//
//  Created by Romain Beguet on 17.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "ASTVisitorTemplate.h"

namespace sfsl {

namespace ast {

ASTVisitorTemplate::ASTVisitorTemplate(std::shared_ptr<common::CompilationContext> &ctx) : ASTVisitor(ctx) {

}

void ASTVisitorTemplate::visit(Program *prog) {

}

void ASTVisitorTemplate::visit(ModuleDecl *module) {

}

void ASTVisitorTemplate::visit(DefineDecl *module) {

}

void ASTVisitorTemplate::visit(BinaryExpression *exp) {

}

void ASTVisitorTemplate::visit(FunctionCall *call) {

}

void ASTVisitorTemplate::visit(Identifier *ident) {

}

void ASTVisitorTemplate::visit(IntLitteral *intlit) {

}

void ASTVisitorTemplate::visit(RealLitteral *reallit) {

}



}

}
