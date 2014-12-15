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

    for (ModuleDecl* mod : prog->getModules()) {
        const std::string& name = mod->getName()->getValue();
        sym::ModuleSymbol* modsym = _mngr.New<sym::ModuleSymbol>(name);

        mod->setSymbol(modsym);
        _curScope->addSymbol(name, modsym);
    }

    ASTVisitor::visit(prog);

    popScope();
}

void ScopeGeneration::visit(ModuleDecl *module) {
    pushScope(module->getSymbol());

    for (ModuleDecl* submod : module->getSubModules()) {
        const std::string& name = submod->getName()->getValue();
        sym::ModuleSymbol* submodsym = _mngr.New<sym::ModuleSymbol>(name);

        submod->setSymbol(submodsym);
        _curScope->addSymbol(name, submodsym);
    }

    for (DefineDecl* decl : module->getDeclarations()) {
        const std::string& name = decl->getName()->getValue();
        sym::DefinitionSymbol* def = _mngr.New<sym::DefinitionSymbol>(name);

        decl->setSymbol(def);
        _curScope->addSymbol(name, def);
    }

    ASTVisitor::visit(module);

    popScope();
}

void ScopeGeneration::visit(DefineDecl *def) {
    pushScope(nullptr, true);



    popScope();
}

void ScopeGeneration::visit(Block *block) {
    pushScope();



    popScope();
}

void ScopeGeneration::visit(FunctionCreation *func) {
    pushScope();



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

}

}
