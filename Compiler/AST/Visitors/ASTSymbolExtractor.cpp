//
//  ASTSymbolExtractor.cpp
//  SFSL
//
//  Created by Romain Beguet on 30.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "ASTSymbolExtractor.h"

namespace sfsl {

namespace ast {

ASTSymbolExtractor::ASTSymbolExtractor(CompCtx_Ptr& ctx) : ASTVisitor(ctx) {

}

ASTSymbolExtractor::~ASTSymbolExtractor() {

}

void ASTSymbolExtractor::visit(ASTNode*) {
    _sym = nullptr;
}

void ASTSymbolExtractor::visit(ModuleDecl* mod) {
    _sym = mod->getSymbol();
}

void ASTSymbolExtractor::visit(ClassDecl* clss) {
    _sym = clss->getSymbol();
}

void ASTSymbolExtractor::visit(DefineDecl* def) {
    _sym = def->getSymbol();
}

void ASTSymbolExtractor::visit(Identifier* id) {
    _sym = id->getSymbol();
}

void ASTSymbolExtractor::visit(MemberAccess* mac) {
    _sym = mac->getSymbol();
}

sym::Symbol* ASTSymbolExtractor::getSymbol() const {
    return _sym;
}

}

}
