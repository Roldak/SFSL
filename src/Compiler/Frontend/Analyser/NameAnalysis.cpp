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

ScopePossessorVisitor::ScopePossessorVisitor(CompCtx_Ptr& ctx) : ASTVisitor(ctx), _curScope(nullptr) {

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
    initCreated(node, sym);
    return sym;
}

sym::DefinitionSymbol* ScopePossessorVisitor::createSymbol(DefineDecl* node, TypeExpression* currentThis) {
    sym::DefinitionSymbol* sym = _mngr.New<sym::DefinitionSymbol>(node->getName()->getValue(), node, currentThis);
    initCreated(node, sym);
    return sym;
}

sym::TypeSymbol* ScopePossessorVisitor::createSymbol(TypeDecl* node) {
    sym::TypeSymbol* sym = _mngr.New<sym::TypeSymbol>(node->getName()->getValue(), node);
    initCreated(node, sym);
    return sym;
}

template<typename T, typename S>
void ScopePossessorVisitor::initCreated(T* id, S* s) {
    s->setPos(*id);
    id->setSymbol(s);
    tryAddSymbol(s);
}

template<typename T>
void ScopePossessorVisitor::setVariableSymbolicUsed(T* symbolic, bool val) {
    if (sym::Symbol* s = symbolic->getSymbol()) {
        if (s->getSymbolType() == sym::SYM_VAR) {
            static_cast<sym::VariableSymbol*>(s)->setUsed(val);
        }
    }
}

// SCOPE GENERATION

ScopeGeneration::ScopeGeneration(CompCtx_Ptr &ctx) : ScopePossessorVisitor(ctx), _currentThis(nullptr) {
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

    SAVE_MEMBER_AND_SET(_currentThis, clss)

    ASTVisitor::visit(clss);

    RESTORE_MEMBER(_currentThis)

    popScope();
}

void ScopeGeneration::visit(DefineDecl* def) {
    createSymbol(def, _currentThis);

    pushScope(def->getSymbol(), true);

    SAVE_MEMBER_AND_SET(_currentThis, nullptr)

    ASTVisitor::visit(def);

    RESTORE_MEMBER(_currentThis)

    popScope();
}

void ScopeGeneration::visit(TypeConstructorCreation* tc) {
    pushScope(tc);

    TypeExpression* expr = tc->getArgs();
    std::vector<TypeExpression*> args;

    if (isNodeOfType<TypeTuple>(expr, _ctx)) { // form is `[] => ...` or `[exp, exp] => ...`, ...
        args = static_cast<TypeTuple*>(expr)->getExpressions();
    } else { // form is `exp => ...` or `[exp] => ...`
        args.push_back(expr);
    }

    for (TypeExpression* expr : args) {
        if (isNodeOfType<TypeIdentifier>(expr, _ctx)) { // arg of the form `T`
            createProperType(static_cast<TypeIdentifier*>(expr),
                             ASTDefaultTypeFromKindCreator::createDefaultTypeFromKind(
                                 _mngr.New<ProperTypeKindSpecifier>(), static_cast<TypeIdentifier*>(expr)->getValue(), _ctx));
        } else if(isNodeOfType<KindSpecifier>(expr, _ctx)) { // arg of the form `x: kind`
            // The type var is already going to be created by the KindSpecifier Node
            expr->onVisit(this);
        } else {
            _ctx->reporter().error(*expr, "Type argument should be an identifier");
        }
    }

    tc->getBody()->onVisit(this);

    popScope();
}

void ScopeGeneration::visit(KindSpecifier* ks) {
    TypeDecl* defaultType = ASTDefaultTypeFromKindCreator::createDefaultTypeFromKind(
                ks->getKindNode(), ks->getSpecified()->getValue(), _ctx);

    createProperType(ks->getSpecified(), defaultType);
}

void ScopeGeneration::visit(Block* block) {
    pushScope(block);

    ASTVisitor::visit(block);

    popScope();
}

void ScopeGeneration::visit(FunctionCreation* func) {
    pushScope(func);

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

    popScope();
}

void ScopeGeneration::visit(TypeSpecifier* tps) {
    tps->getTypeNode()->onVisit(this);
    createVar(tps->getSpecified());
    setVariableSymbolicUsed(tps->getSpecified(), false);
}

void ScopeGeneration::createVar(Identifier* id) {
    sym::VariableSymbol* arg = _mngr.New<sym::VariableSymbol>(id->getValue());
    initCreated(id, arg);
}

void ScopeGeneration::createProperType(TypeIdentifier* id, TypeDecl* defaultType) {
    initCreated(id, defaultType->getSymbol());
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

// TYPE DEPENDENCY FIXATION


TypeDependencyFixation::TypeDependencyFixation(CompCtx_Ptr& ctx) : ScopePossessorVisitor(ctx) {

}

TypeDependencyFixation::~TypeDependencyFixation() {

}

void TypeDependencyFixation::visit(ModuleDecl* mod) {
    SAVE_SCOPE(mod->getSymbol())

    ASTVisitor::visit(mod);

    RESTORE_SCOPE
}

void TypeDependencyFixation::visit(TypeDecl* tdecl) {
    SAVE_SCOPE(tdecl->getSymbol())

    ASTVisitor::visit(tdecl);

    RESTORE_SCOPE
}

void TypeDependencyFixation::visit(ClassDecl* clss) {
    SAVE_SCOPE(clss)

    _parametrizables.push_back(clss);

    ASTVisitor::visit(clss);

    _parametrizables.pop_back();

    RESTORE_SCOPE
}

void TypeDependencyFixation::visit(DefineDecl* def) {
    SAVE_SCOPE(def->getSymbol())

    ASTVisitor::visit(def);

    RESTORE_SCOPE
}

void TypeDependencyFixation::visit(TypeConstructorCreation* tc) {
    SAVE_SCOPE(tc)

    _parametrizables.push_back(tc);

    ASTVisitor::visit(tc);

    _parametrizables.pop_back();

    RESTORE_SCOPE
}

void TypeDependencyFixation::visit(Block* block) {
    SAVE_SCOPE(block)

    ASTVisitor::visit(block);

    RESTORE_SCOPE
}

void TypeDependencyFixation::visit(FunctionCreation* func) {
    SAVE_SCOPE(func)

    _parametrizables.push_back(func);

    ASTVisitor::visit(func);

    _parametrizables.pop_back();

    RESTORE_SCOPE
}

void TypeDependencyFixation::visit(TypeIdentifier* id) {
    if (sym::TypeSymbol* ts = _curScope->getSymbol<sym::TypeSymbol>(id->getValue())) {
        std::cerr << ts->isTypeVariable() << std::endl;
        if (ts->isTypeVariable()) {
            updateAllParametrizable(ts);
        }
    }
}

void TypeDependencyFixation::updateAllParametrizable(sym::TypeSymbol* ts) {
    for (type::TypeParametrizable* tp : _parametrizables) {
        tp->setDependsOn(ts);
    }
}

template<typename T>
void TypeDependencyFixation::debugDumpDependencies(const T* param) const {
    if (param->getDependencies().size() > 0) {
        std::string acc = param->getName() + " depends on {";

        for (sym::TypeSymbol* t : param->getDependencies()) {
            acc += t->getName() + ", ";
        }

        acc = acc.substr(0, acc.size() - 2);

        _ctx->reporter().info(*param, acc + "}");
    }
}

// SYMBOL ASSIGNATION

SymbolAssignation::SymbolAssignation(CompCtx_Ptr& ctx) : ScopePossessorVisitor(ctx) {
}

SymbolAssignation::~SymbolAssignation() {

}

void SymbolAssignation::visit(ModuleDecl* mod) {
    SAVE_SCOPE(mod->getSymbol())

    ASTVisitor::visit(mod);

    RESTORE_SCOPE
}

void SymbolAssignation::visit(TypeDecl* tdecl) {
    SAVE_SCOPE(tdecl->getSymbol())

    ASTVisitor::visit(tdecl);

    RESTORE_SCOPE
}

void SymbolAssignation::visit(ClassDecl* clss) {
    if (TRY_INSERT(_visitedTypes, clss)) {
        SAVE_SCOPE(clss)

        if (clss->getParent()) {
            clss->getParent()->onVisit(this);
            if (type::Type* parentType = ASTTypeCreator::createType(clss->getParent(), _ctx)) {
                if (type::ProperType* parent = type::getIf<type::ProperType>(parentType->applied(_ctx))) {
                    ClassDecl* parentClass = parent->getClass();
                    parentClass->onVisit(this);

                    _curScope->copySymbolsFrom(parentClass->getScope(), parent->getSubstitutionTable());

                    addSubtypeRelations(clss, parentClass);
                }
            }
        }

        updateSubtypeRelations(clss);

        for (TypeSpecifier* field : clss->getFields()) {
            field->onVisit(this);
        }
        for (DefineDecl* def : clss->getDefs()) {
            def->onVisit(this);
        }

        RESTORE_SCOPE
    }
}

void SymbolAssignation::visit(DefineDecl* def) {
    SAVE_SCOPE(def->getSymbol())

    ASTVisitor::visit(def);

    RESTORE_SCOPE
}

void SymbolAssignation::visit(TypeMemberAccess* tmac) {
    assignMemberAccess(tmac);
}

void SymbolAssignation::visit(TypeConstructorCreation* tc) {
    SAVE_SCOPE(tc)

    ASTVisitor::visit(tc);

    RESTORE_SCOPE
}

void SymbolAssignation::visit(TypeIdentifier* id) {
    assignIdentifier(id);
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

    ASTVisitor::visit(func);

    warnForUnusedVariableInCurrentScope();

    RESTORE_SCOPE
}

void SymbolAssignation::visit(Identifier* id) {
    assignIdentifier(id);
    setVariableSymbolicUsed(id, true);
}

template<typename T>
void SymbolAssignation::assignIdentifier(T* id) {
    if (!_curScope->assignSymbolic<sym::Symbol>(*id, id->getValue())) {
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

    if (!scope->assignSymbolic<sym::Symbol>(*mac, id)) {
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

void SymbolAssignation::addSubtypeRelations(ClassDecl* clss, ClassDecl* parent) {
    clss->CanSubtypeClasses::insertParents(parent->CanSubtypeClasses::cParentBegin(), parent->CanSubtypeClasses::cParentEnd());
}

void SymbolAssignation::updateSubtypeRelations(ClassDecl* clss) {
    clss->CanSubtypeClasses::insertParent(clss);
    clss->CanSubtypeClasses::updateParents();
}

void SymbolAssignation::warnForUnusedVariableInCurrentScope() {
    for (const auto& s : _curScope->getAllSymbols()) {
        if (s.second.symbol->getSymbolType() == sym::SYM_VAR) {
            sym::VariableSymbol* var = static_cast<sym::VariableSymbol*>(s.second.symbol);
            if (!var->isUsed()) {
                _ctx->reporter().warning(*var, "Unused variable '" + var->getName() + "'");
            }
        }
    }
}

}

}
