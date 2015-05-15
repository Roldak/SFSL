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
#include "../AST/Visitors/ASTKindCreator.h"

namespace sfsl {

namespace ast {

// SCOPE POSSESSOR VISITOR

ScopePossessorVisitor::ScopePossessorVisitor(CompCtx_Ptr &ctx) : ASTVisitor(ctx), _curScope(nullptr) {

}

ScopePossessorVisitor::~ScopePossessorVisitor() {

}

void ScopePossessorVisitor::tryAddSymbol(sym::Symbol* sym) {
    if (sym::Symbol* oldSymbol = _curScope->addSymbol(sym)) {
        _ctx->reporter().error(*sym, std::string("Multiple definitions of symbol '") +
                                     sym->getName() + "' were found.");
        _ctx->reporter().info(*oldSymbol, "First instance here");
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

    pushScope(tdecl->getSymbol(), true);

    ASTVisitor::visit(tdecl);

    popScope();
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

void SymbolAssignation::visit(TypeDecl *tdecl) {
    SAVE_SCOPE(tdecl->getSymbol())

    ASTVisitor::visit(tdecl);

    RESTORE_SCOPE
}

void SymbolAssignation::visit(TypeMemberAccess* tmac) {
    assignMemberAccess(tmac);
}

void SymbolAssignation::visit(TypeConstructorCreation* tc) {
    SAVE_SCOPE(tc)

    TypeExpression* expr = tc->getArgs();
    std::vector<TypeExpression*> args;

    if (isNodeOfType<TypeTuple>(expr, _ctx)) { // form is `[] => ...` or `[exp, exp] => ...`, ...
        args = static_cast<TypeTuple*>(expr)->getExpressions();
    } else { // form is `exp => ...` or `[exp] => ...`
        args.push_back(expr);
    }

    for (TypeExpression* expr : args) {
        if (isNodeOfType<TypeIdentifier>(expr, _ctx)) { // arg of the form `x`
            createObjectType(static_cast<TypeIdentifier*>(expr),
                             ASTDefaultTypeFromKindCreator::createDefaultTypeFromKind(
                                 _mngr.New<ProperTypeKindSpecifier>(), static_cast<TypeIdentifier*>(expr)->getValue(), _ctx));
        } else if(isNodeOfType<KindSpecifier>(expr, _ctx)) { // arg of the form `x: type`
            // The var is already going to be created by the KindSpecifier Node
            expr->onVisit(this);
        } else {
            _ctx->reporter().error(*expr, "Type argument should be an identifier");
        }
    }

    tc->getBody()->onVisit(this);

    RESTORE_SCOPE
}

void SymbolAssignation::visit(TypeIdentifier* id) {
    assignIdentifier(id);
}

void SymbolAssignation::visit(KindSpecifier* ks) {
    TypeDecl* defaultType = ASTDefaultTypeFromKindCreator::createDefaultTypeFromKind(
                ks->getKindNode(), ks->getSpecified()->getValue(), _ctx);

    createObjectType(ks->getSpecified(), defaultType);
    //ASTVisitor::visit(ks);
}

void SymbolAssignation::visit(BinaryExpression* exp) {
    exp->getLhs()->onVisit(this);
    exp->getRhs()->onVisit(this);
}

void SymbolAssignation::visit(MemberAccess* mac) {
    assignMemberAccess(mac);
}

void SymbolAssignation::visit(Block* block) {
    SAVE_SCOPE(block)

    ASTVisitor::visit(block);

    warnForUnusedVariableInCurrentScope();
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
            _ctx->reporter().error(*expr, "Argument should be an identifier");
        }
    }

    func->getBody()->onVisit(this);

    warnForUnusedVariableInCurrentScope();
    RESTORE_SCOPE
}

void SymbolAssignation::visit(TypeSpecifier* tps) {
    createVar(tps->getSpecified());
    ASTVisitor::visit(tps);
    setVariableSymbolicUsed(tps->getSpecified(), false);
}

void SymbolAssignation::visit(Identifier* id) {
    assignIdentifier(id);
    setVariableSymbolicUsed(id, true);
}

void SymbolAssignation::createVar(Identifier* id) {
    sym::VariableSymbol* arg = _mngr.New<sym::VariableSymbol>(id->getValue());
    initCreated(id, arg);
}

void SymbolAssignation::createObjectType(TypeIdentifier* id, TypeDecl* defaultType) {
    initCreated(id, defaultType->getSymbol());
}

template<typename T, typename S>
void SymbolAssignation::initCreated(T* id, S* s) {
    s->setPos(*id);
    id->setSymbol(s);
    tryAddSymbol(s);
}

template<typename T>
void SymbolAssignation::assignIdentifier(T* id) {
    if (sym::Symbol* symbol = _curScope->getSymbol<sym::Symbol>(id->getValue())) {
        id->setSymbol(symbol);
    } else {
        _ctx->reporter().error(*id, "Undefined symbol '" + id->getValue() + "'");
    }
}

template<typename T>
void SymbolAssignation::assignMemberAccess(T* mac) {
    mac->getAccessed()->onVisit(this);

    if (sym::Symbol* sym = ASTSymbolExtractor::extractSymbol(mac->getAccessed(), _ctx)) {
        switch (sym->getSymbolType()) {
        case sym::SYM_MODULE:   assignFromStaticScope(mac, static_cast<sym::ModuleSymbol*>(sym), "module " + sym->getName()); break;
        case sym::SYM_TPE:      assignFromTypeSymbol(mac, static_cast<sym::TypeSymbol*>(sym)); break;
        default:
            mac->setSymbol(nullptr);
            break;
        }
    }
}

template<typename T>
void SymbolAssignation::assignFromStaticScope(T* mac, sym::Scoped* scoped, const std::string& typeName) {
    sym::Scope* scope = scoped->getScope();
    const std::string& id = mac->getMember()->getValue();

    if (sym::Symbol* resSymbol = scope->getSymbol<sym::Symbol>(id, false)) {
        mac->setSymbol(resSymbol);
    } else {
        _ctx->reporter().error(
                    *(mac->getMember()),
                    std::string("No member named '") + id + "' in " + typeName);
    }
}

template<typename T>
void SymbolAssignation::assignFromTypeSymbol(T* mac, sym::TypeSymbol* tsym) {
    if (ClassDecl* clss = getClassDeclFromTypeSymbol(tsym, _ctx)) {
        assignFromStaticScope<T>(mac, clss, "class " + clss->getName());
    } else {
        _ctx->reporter().error(*mac->getMember(), "Type " + tsym->getName() + " cannot have any members");
    }
}

template<typename T>
void SymbolAssignation::setVariableSymbolicUsed(T* symbolic, bool val) {
    if (sym::Symbol* s = symbolic->getSymbol()) {
        if (s->getSymbolType() == sym::SYM_VAR) {
            static_cast<sym::VariableSymbol*>(s)->setUsed(val);
        }
    }
}

void SymbolAssignation::warnForUnusedVariableInCurrentScope() {
    for (const auto& s : _curScope->getAllSymbols()) {
        if (s.second->getSymbolType() == sym::SYM_VAR) {
            sym::VariableSymbol* var = static_cast<sym::VariableSymbol*>(s.second);
            if (!var->isUsed()) {
                _ctx->reporter().warning(*var, "Unused variable '" + var->getName() + "'");
            }
        }
    }
}

}

}
