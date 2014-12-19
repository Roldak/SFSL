//
//  ASTVisitorTemplate.cpp
//  SFSL
//
//  Created by Romain Beguet on 17.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "NameAnalysis.h"
#include "../AST/Symbols/Scope.h"
#include "../AST/Symbols/Symbols.h"

namespace sfsl {

namespace ast {

ScopeGeneration::ScopeGeneration(std::shared_ptr<common::CompilationContext> &ctx) : ASTVisitor(ctx), _curScope(nullptr) {

}

void ScopeGeneration::visit(Program *prog) {
    pushScope();

    ASTVisitor::visit(prog);

    popScope();
}

void ScopeGeneration::visit(ModuleDecl *module) {
    createSymbol<sym::ModuleSymbol>(module);

    pushScope(module->getSymbol());

    ASTVisitor::visit(module);

    popScope();
}

void ScopeGeneration::visit(DefineDecl *def) {
    createSymbol<sym::DefinitionSymbol>(def);

    pushScope(nullptr, true);

    ASTVisitor::visit(def);

    popScope();
}

void ScopeGeneration::visit(Block *block) {
    pushScope();

    ASTVisitor::visit(block);

    popScope();
}

void ScopeGeneration::visit(FunctionCreation *func) {
    pushScope();

    ASTVisitor::visit(func);

    popScope();
}

void ScopeGeneration::pushScope(sym::Scoped *scoped, bool isDefScope) {
    _curScope = _mngr.New<sym::Scope>(_curScope, isDefScope);
    if (scoped != nullptr) {
        scoped->setScope(_curScope);
    }
}

void ScopeGeneration::popScope() {
    _curScope = _curScope->getParent();
}

void ScopeGeneration::tryAddSymbol(sym::Symbol *sym) {
    if (sym::Symbol* oldSymbol = _curScope->addSymbol(sym)) {
        _ctx.get()->reporter().error(*sym,
                                     std::string("Multiple definitions of symbol '") + sym->getName() +
                                     "' were found. First instance here : " +
                                     oldSymbol->positionStr());
    }
}

template<typename T, typename U>
T* ScopeGeneration::createSymbol(U* node) {
    T* sym = _mngr.New<T>(node->getName()->getValue());
    sym->setPos(*node);

    node->setSymbol(sym);
    tryAddSymbol(sym);

    return sym;
}

}

}
