//
//  TypeChecking.cpp
//  SFSL
//
//  Created by Romain Beguet on 30.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "KindChecking.h"
#include "../AST/Visitors/ASTTypeIdentifier.h"
#include "../AST/Visitors/ASTTypeCreator.h"
#include "../AST/Symbols/Scope.h"

namespace sfsl {

namespace ast {

// TYPE CHECK

KindChecking::KindChecking(CompCtx_Ptr& ctx)
    : ASTVisitor(ctx), _rep(ctx.get()->reporter()) {

}

KindChecking::~KindChecking() {

}

void KindChecking::visit(ASTNode*) {

}

void KindChecking::visit(TypeDecl* tdecl) {

}

void KindChecking::visit(ClassDecl* clss) {

}

void KindChecking::visit(TypeMemberAccess* tdot) {

}

void KindChecking::visit(TypeTuple* ttuple) {

}

void KindChecking::visit(TypeConstructorCreation* typeconstructor) {

}

void KindChecking::visit(TypeConstructorCall* tcall) {

}

void KindChecking::visit(TypeIdentifier* tident) {

}

}

}
