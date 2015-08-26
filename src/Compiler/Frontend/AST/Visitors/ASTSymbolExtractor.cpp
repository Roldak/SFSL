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

ASTSymbolExtractor::ASTSymbolExtractor(CompCtx_Ptr& ctx)
    : ASTExplicitVisitor(ctx), _sym(nullptr) {

}

ASTSymbolExtractor::~ASTSymbolExtractor() {

}

void ASTSymbolExtractor::visit(ASTNode*) {
    // do not throw an exception
}

void ASTSymbolExtractor::visit(ModuleDecl* mod) {
    _sym = mod->getSymbol();
}

void ASTSymbolExtractor::visit(TypeDecl* type) {
    _sym = type->getSymbol();
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

void ASTSymbolExtractor::visit(TypeIdentifier *id) {
    _sym = id->getSymbol();
}

void ASTSymbolExtractor::visit(TypeMemberAccess *mac) {
    _sym = mac->getSymbol();
}

sym::Symbol* ASTSymbolExtractor::getSymbol() const {
    return _sym;
}

sym::Symbol* ASTSymbolExtractor::extractSymbol(ASTNode* node, CompCtx_Ptr& ctx) {
    ASTSymbolExtractor extractor(ctx);
    node->onVisit(&extractor);
    return extractor.getSymbol();
}

}

}
