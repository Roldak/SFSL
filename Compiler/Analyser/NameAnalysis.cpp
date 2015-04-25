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
#include "../AST/Visitors/ASTTypeCreator.h"

namespace sfsl {

namespace ast {

// SCOPE POSSESSOR VISITOR

ScopePossessorVisitor::ScopePossessorVisitor(CompCtx_Ptr &ctx) : ASTVisitor(ctx), _curScope(nullptr) {

}

ScopePossessorVisitor::~ScopePossessorVisitor() {

}

void ScopePossessorVisitor::tryAddSymbol(sym::Symbol* sym) {
    if (sym::Symbol* oldSymbol = _curScope->addSymbol(sym)) {
        _ctx.get()->reporter().error(*sym, std::string("Multiple definitions of symbol '") +
                                     sym->getName() + "' were found.");
        _ctx.get()->reporter().info(*oldSymbol, "First instance here");
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

sym::DefinitionSymbol* ScopePossessorVisitor::createSymbol(DefineDecl *node) {
    sym::DefinitionSymbol* sym = _mngr.New<sym::DefinitionSymbol>(node->getName()->getValue(), node);
    sym->setPos(*node);

    node->setSymbol(sym);
    tryAddSymbol(sym);

    return sym;
}

sym::TypeSymbol* ScopePossessorVisitor::createSymbol(TypeDecl* node) {
    sym::TypeSymbol* sym = _mngr.New<sym::TypeSymbol>(node->getName()->getValue(), node);
    sym->setPos(*node);

    node->setSymbol(sym);
    tryAddSymbol(sym);

    return sym;
}

// SCOPE GENERATION

ScopeGeneration::ScopeGeneration(CompCtx_Ptr &ctx) : ScopePossessorVisitor(ctx) {
}

ScopeGeneration::~ScopeGeneration() {

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

void ScopeGeneration::visit(TypeDecl* tdecl) {
    createSymbol(tdecl);

    ASTVisitor::visit(tdecl);
}

void ScopeGeneration::visit(ClassDecl* clss) {
    pushScope(clss, true);

    ASTVisitor::visit(clss);

    popScope();
}

void ScopeGeneration::visit(DefineDecl* def) {
    createSymbol(def);

    pushScope(def->getSymbol(), true);

    ASTVisitor::visit(def);

    popScope();
}

void ScopeGeneration::visit(TypeConstructorCreation* typeconstructor) {
    pushScope(typeconstructor);

    ASTVisitor::visit(typeconstructor);

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

SymbolAssignation::SymbolAssignation(CompCtx_Ptr &ctx) : ScopePossessorVisitor(ctx) {
}

SymbolAssignation::~SymbolAssignation() {

}

void SymbolAssignation::visit(ModuleDecl* mod) {
    SAVE_SCOPE(mod->getSymbol())

    ASTVisitor::visit(mod);

    RESTORE_SCOPE
}

void SymbolAssignation::visit(ClassDecl *clss) {
    SAVE_SCOPE(clss)

    ASTVisitor::visit(clss);

    RESTORE_SCOPE
}

void SymbolAssignation::visit(DefineDecl* def) {
    SAVE_SCOPE(def->getSymbol())

    ASTVisitor::visit(def);

    RESTORE_SCOPE
}

void SymbolAssignation::visit(TypeConstructorCreation* typeconstructor) {
    SAVE_SCOPE(typeconstructor)

    const std::vector<Expression*>& args = typeconstructor->getArgs()->getExpressions();

    for (Expression* expr : args) {
        if (isNodeOfType<Identifier>(expr, _ctx)) { // arg of the form `x`
            createObjectType(static_cast<Identifier*>(expr));
        }
        else if(isNodeOfType<TypeConstructorCall>(expr, _ctx)) {
            TypeConstructorCall* call = static_cast<TypeConstructorCall*>(expr);
            createTypeConstructor(static_cast<Identifier*>(call->getCallee()), call->getArgsTuple()); // TODO : safer cast
        }
        else {
            _ctx.get()->reporter().error(*expr, "Argument should be an identifier");
        }
    }

    typeconstructor->getBody()->onVisit(this);

    RESTORE_SCOPE
}

void SymbolAssignation::visit(BinaryExpression* exp) {
    exp->getLhs()->onVisit(this);
    exp->getRhs()->onVisit(this);
}

void SymbolAssignation::visit(MemberAccess* mac) {
    mac->getAccessed()->onVisit(this);

    if (sym::Symbol* sym = extractSymbol(mac->getAccessed(), _ctx)) {
        switch (sym->getSymbolType()) {
        case sym::SYM_MODULE:   assignFromStaticScope(mac, static_cast<sym::ModuleSymbol*>(sym), "module " + sym->getName()); break;
        case sym::SYM_TPE:      assignFromTypeSymbol(mac, static_cast<sym::TypeSymbol*>(sym)); break;
        default:
            mac->setSymbol(nullptr);
            break;
        }
    }
}

void SymbolAssignation::visit(Block* block) {
    SAVE_SCOPE(block)

    ASTVisitor::visit(block);

    RESTORE_SCOPE
}

void SymbolAssignation::visit(FunctionCreation* func) {
    SAVE_SCOPE(func)

    Expression* expr = func->getArgs();
    std::vector<Expression*> args;

    if (isNodeOfType<Tuple>(expr, _ctx)) { // form is `() => ...` or `(exp, exp) => ...`, ...
        args = static_cast<Tuple*>(expr)->getExpressions();
    } else { // form is `exp => ...` or `(exp) => ...`
        args.push_back(expr);
    }

    for (Expression* expr : args) {
        if (isNodeOfType<Identifier>(expr, _ctx)) { // arg of the form `x`
            createVar(static_cast<Identifier*>(expr));
        } else if(isNodeOfType<TypeSpecifier>(expr, _ctx)) { // arg of the form `x: type`
            // The var is already going to be created by the TypeSpecifier Node
            expr->onVisit(this);
        } else {
            _ctx.get()->reporter().error(*expr, "Argument should be an identifier");
        }
    }

    func->getBody()->onVisit(this);

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
    initCreated(id, arg);
}

void SymbolAssignation::createObjectType(Identifier *id) {
    ClassDecl* clss = _mngr.New<ClassDecl>(id->getValue(), std::vector<TypeSpecifier*>(), std::vector<DefineDecl*>());
    clss->setScope(_mngr.New<sym::Scope>(nullptr));
    TypeDecl* type = _mngr.New<TypeDecl>(id, clss);
    sym::TypeSymbol* arg = _mngr.New<sym::TypeSymbol>(id->getValue(), type);
    arg->setType(createType(clss, _ctx));
    initCreated(id, arg);
}

void SymbolAssignation::createTypeConstructor(Identifier *id, TypeTuple *ttuple) {
    for (Expression* expr : ttuple->getExpressions()) {
        createObjectType(static_cast<Identifier*>(expr));
    }
    ClassDecl* resClass = _mngr.New<ClassDecl>(id->getValue(), std::vector<TypeSpecifier*>(), std::vector<DefineDecl*>());
    resClass->setScope(_mngr.New<sym::Scope>(nullptr));
    TypeConstructorCreation* typeconstuctor = _mngr.New<TypeConstructorCreation>(id->getValue(), ttuple, resClass);

    TypeDecl* type = _mngr.New<TypeDecl>(id, typeconstuctor);
    sym::TypeSymbol* arg = _mngr.New<sym::TypeSymbol>(id->getValue(), type);
    arg->setType(createType(typeconstuctor, _ctx));
    initCreated(id, arg);
}

void SymbolAssignation::initCreated(Identifier *id, sym::Symbol *s) {
    s->setPos(*id);
    id->setSymbol(s);
    tryAddSymbol(s);
}

void SymbolAssignation::assignFromStaticScope(MemberAccess* mac, sym::Scoped* scoped, const std::string& typeName) {
    sym::Scope* scope = scoped->getScope();
    const std::string& id = mac->getMember()->getValue();

    if (sym::Symbol* resSymbol = scope->getSymbol<sym::Symbol>(id, false)) {
        mac->setSymbol(resSymbol);
    } else {
        _ctx.get()->reporter().error(
                    *(mac->getMember()),
                    std::string("No member named '") + id + "' in " + typeName);
    }
}

void SymbolAssignation::assignFromTypeSymbol(MemberAccess* mac, sym::TypeSymbol* tsym) {
    if (ClassDecl* clss = getClassDeclFromTypeSymbol(tsym, _ctx)) {
        assignFromStaticScope(mac, clss, "class " + clss->getName());
    } else {
        _ctx.get()->reporter().error(*mac->getMember(), "Type " + tsym->getName() + " cannot have any members");
    }
}

}

}
