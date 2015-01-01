//
//  NameAnalysis.cpp
//  SFSL
//
//  Created by Romain Beguet on 13.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "NameAnalysis.h"
#include "../AST/Symbols/Scope.h"
#include "../AST/Symbols/Symbols.h"
#include "../AST/Visitors/ASTTypeIdentifier.h"
#include "../AST/Visitors/ASTSymbolExtractor.h"

namespace sfsl {

namespace ast {

// SCOPE POSSESSOR VISITOR

ScopePossessorVisitor::ScopePossessorVisitor(std::shared_ptr<common::CompilationContext> &ctx) : ASTVisitor(ctx), _curScope(nullptr) {

}

void ScopePossessorVisitor::tryAddSymbol(sym::Symbol* sym) {
    if (sym::Symbol* oldSymbol = _curScope->addSymbol(sym)) {
        _ctx.get()->reporter().error(*sym,
                                     std::string("Multiple definitions of symbol '") + sym->getName() +
                                     "' were found. First instance here : " +
                                     oldSymbol->positionStr());
    }
}

template<typename T, typename U>
T* ScopePossessorVisitor::createSymbol(U* node) {
    T* sym = _mngr.New<T>(node->getName()->getValue());
    sym->setPos(*node);

    node->setSymbol(sym);
    tryAddSymbol(sym);

    return sym;
}

// SCOPE GENERATION

ScopeGeneration::ScopeGeneration(std::shared_ptr<common::CompilationContext> &ctx) : ScopePossessorVisitor(ctx) {

}

void ScopeGeneration::visit(Program* prog) {
    pushScope(prog);

    ASTVisitor::visit(prog);

    popScope();
}

void ScopeGeneration::visit(ModuleDecl* module) {
    if (sym::ModuleSymbol* mod = _curScope->getSymbol<sym::ModuleSymbol>(module->getName()->getValue(), false)) {
        module->setSymbol(mod);

        sym::Scope* last = _curScope;
        _curScope = mod->getScope();

        ASTVisitor::visit(module);

        _curScope = last;

    } else {
        createSymbol<sym::ModuleSymbol>(module);

        pushScope(module->getSymbol());

        ASTVisitor::visit(module);

        popScope();
    }
}

void ScopeGeneration::visit(ClassDecl* clss) {
    createSymbol<sym::ClassSymbol>(clss);

    pushScope(clss->getSymbol(), true);

    ASTVisitor::visit(clss);

    popScope();
}

void ScopeGeneration::visit(DefineDecl* def) {
    createSymbol<sym::DefinitionSymbol>(def);

    pushScope(def->getSymbol(), true);

    ASTVisitor::visit(def);

    popScope();
}

void ScopeGeneration::visit(Block* block) {
    pushScope(block);

    ASTVisitor::visit(block);

    popScope();
}

void ScopeGeneration::visit(FunctionCreation* func) {
    pushScope(func);

    ASTVisitor::visit(func);

    popScope();
}

void ScopeGeneration::pushScope(sym::Scoped* scoped, bool isDefScope) {
    _curScope = _mngr.New<sym::Scope>(_curScope, isDefScope);
    if (scoped != nullptr) {
        scoped->setScope(_curScope);
    }
}

void ScopeGeneration::popScope() {
    _curScope = _curScope->getParent();
}

// SYMBOL ASSIGNATION

#define SAVE_SCOPE  sym::Scope* last = _curScope;
#define RESTORE_SCOPE _curScope = last;

SymbolAssignation::SymbolAssignation(std::shared_ptr<common::CompilationContext> &ctx) : ScopePossessorVisitor(ctx) {

}

void SymbolAssignation::visit(ModuleDecl* mod) {
    SAVE_SCOPE
    _curScope = mod->getSymbol()->getScope();

    ASTVisitor::visit(mod);

    RESTORE_SCOPE
}

void SymbolAssignation::visit(ClassDecl *clss) {
    SAVE_SCOPE
    _curScope = clss->getSymbol()->getScope();

    ASTVisitor::visit(clss);

    RESTORE_SCOPE
}

void SymbolAssignation::visit(DefineDecl* def) {
    SAVE_SCOPE
    _curScope = def->getSymbol()->getScope();

    ASTVisitor::visit(def);

    RESTORE_SCOPE
}

void SymbolAssignation::visit(BinaryExpression* exp) {
    exp->getLhs()->onVisit(this);
    exp->getRhs()->onVisit(this);
}

void SymbolAssignation::visit(MemberAccess* mac) {
    mac->getAccessed()->onVisit(this);
    if (sym::Symbol* sym = extractSymbol(mac->getAccessed(), _ctx)) {
        if (sym->getSymbolType() == sym::SYM_MODULE) {

            sym::Scope* scope = ((sym::ModuleSymbol*)sym)->getScope();
            const std::string& id = mac->getMember()->getValue();

            if (sym::Symbol* resSymbol = scope->getSymbol<sym::Symbol>(id, false)) {
                mac->setSymbol(resSymbol);
            } else {
                _ctx.get()->reporter().error(
                            *(mac->getMember()),
                            std::string("No member named '") + id + "' in module '" + sym->getName() + "'");
            }
        }
    }
}

void SymbolAssignation::visit(Block* block) {
    SAVE_SCOPE
    _curScope = block->getScope();

    ASTVisitor::visit(block);

    RESTORE_SCOPE
}

void SymbolAssignation::visit(FunctionCreation* func) {
    SAVE_SCOPE
    _curScope = func->getScope();

    Expression* expr = func->getArgs();
    std::vector<Expression*> args;

    if (isNodeOfType<Tuple>(expr, _ctx)) { // form is `() => ...`, `(exp, exp) => ...`
        Tuple* tuple = static_cast<Tuple*>(expr);
        args = tuple->getExpressions();
    } else { // form is `exp => ...`
        args.push_back(expr);
    }

    for (Expression* expr : args) {
        if (isNodeOfType<Identifier>(expr, _ctx)) { // arg of the form `x`
            createVar(static_cast<Identifier*>(expr));
        } else if(!isNodeOfType<TypeSpecifier>(expr, _ctx)) { // arg of the form `x: type`
            _ctx.get()->reporter().error(*expr, "Argument should be an identifier");
        }
    }

    ASTVisitor::visit(func);

    RESTORE_SCOPE
}

void SymbolAssignation::visit(TypeSpecifier* tps) {
    createVar(tps->getSpecified());
    ASTVisitor::visit(tps);
}

void SymbolAssignation::visit(Identifier* id) {
    if (sym::Symbol* symbol = _curScope->getSymbol<sym::Symbol>(id->getValue())) {
        id->setSymbol(symbol);
    } else {
        _ctx.get()->reporter().error(*id, "Undefined symbol '" + id->getValue() + "'");
    }
}

void SymbolAssignation::createVar(Identifier *id) {
    sym::VariableSymbol* arg = _mngr.New<sym::VariableSymbol>(id->getValue());
    arg->setPos(*id);
    id->setSymbol(arg);
    tryAddSymbol(arg);
}

}

}
