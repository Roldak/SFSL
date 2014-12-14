//
//  ASTVisitorTemplate.cpp
//  SFSL
//
//  Created by Romain Beguet on 17.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "NameAnalysis.h"

namespace sfsl {

namespace ast {

ScopeGeneration::ScopeGeneration(std::shared_ptr<common::CompilationContext> &ctx) : ASTVisitor(ctx) {

}

void ScopeGeneration::visit(Program *prog) {

}

void ScopeGeneration::visit(ModuleDecl *module) {

}

void ScopeGeneration::visit(DefineDecl *module) {

}

void ScopeGeneration::visit(Block *block) {

}

}

}
