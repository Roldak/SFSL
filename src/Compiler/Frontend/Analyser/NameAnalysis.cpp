//
//  NameAnalysis.cpp
//  SFSL
//
//  Created by Romain Beguet on 13.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "NameAnalysis.h"
#include "../Symbols/Scope.h"
#include "../Symbols/Symbols.h"
#include "../AST/Visitors/ASTTypeIdentifier.h"
#include "../AST/Visitors/ASTSymbolExtractor.h"
#include "../AST/Visitors/ASTTypeCreator.h"
#include "../AST/Visitors/ASTKindCreator.h"
#include "../AST/Visitors/ASTExpr2TypeExpr.h"

#define SAVE_SCOPE(expr)  \
    sym::Scope* __last_scope__ = _curScope; \
    _curScope = (expr)->getScope();
#define RESTORE_SCOPE _curScope = __last_scope__;

//#define DEBUG_DEPENDENCIES

namespace sfsl {

namespace ast {

const size_t size_t_max = std::numeric_limits<size_t>::max();

// SCOPE GENERATION

ScopeGeneration::ScopeGeneration(CompCtx_Ptr &ctx)
    : ASTImplicitVisitor(ctx), _curScope(nullptr), _invalidFrom(size_t_max), _currentThis(nullptr) {
}

ScopeGeneration::~ScopeGeneration() {

}

void ScopeGeneration::visit(Program* prog) {
    pushScope(prog);

    ASTImplicitVisitor::visit(prog);

    popScope();
}

void ScopeGeneration::visit(ModuleDecl* module) {
    if (sym::ModuleSymbol* mod = _curScope->getSymbol<sym::ModuleSymbol>(module->getName()->getValue(), false)) {
        module->setSymbol(mod);

        sym::Scope* last = _curScope;
        _curScope = mod->getScope();

        pushPathPart(module->getName()->getValue(), false);

        ASTImplicitVisitor::visit(module);

        popPathPart();

        _curScope = last;
    } else {
        createSymbol<sym::ModuleSymbol>(module);

        pushScope(module->getSymbol());
        pushPathPart(module->getName()->getValue(), false);

        ASTImplicitVisitor::visit(module);

        popPathPart();
        popScope();
    }
}

void ScopeGeneration::visit(TypeDecl* tdecl) {
    createSymbol(tdecl);
    if (tdecl->isExtern() && !isValidAbsolutePath()) {
        reportInvalidExternUsage(*tdecl);
    }

    pushScope(tdecl->getSymbol());

    ASTImplicitVisitor::visit(tdecl);

    popScope();
}

void ScopeGeneration::visit(ClassDecl* clss) {
    pushScope(clss, _currentThis != nullptr);
    pushPathPart(clss->getName(), false);

    SAVE_MEMBER_AND_SET(_currentThis, clss)

    ASTImplicitVisitor::visit(clss);

    RESTORE_MEMBER(_currentThis)

    popPathPart();
    popScope();
}

void ScopeGeneration::visit(DefineDecl* def) {
    createSymbol(def, _currentThis);
    if (def->isExtern() && !isValidAbsolutePath()) {
        reportInvalidExternUsage(*def);
    }

    pushScope(def->getSymbol(), _currentThis == nullptr);
    pushPathPart(def->getName()->getValue(), true);

    if (_currentThis) {
        if (TypeExpression* texpr = def->getTypeSpecifier()) {
            if (FunctionTypeDecl* ftdecl = getIfNodeOfType<FunctionTypeDecl>(texpr, _ctx)) {
                _nextMethodDecl = ftdecl;
            }
        }
    }

    SAVE_MEMBER_AND_SET(_currentThis, nullptr)

    // check that the RHS of a constructor definition is a function creation
    if (def->getName()->getValue() == "new" &&
        def->getValue() &&
        !isNodeOfType<FunctionCreation>(def->getValue(), _ctx)) {
        _ctx->reporter().error(*def, "A constructor must be a function");
    }

    ASTImplicitVisitor::visit(def);

    RESTORE_MEMBER(_currentThis)

    popPathPart();
    popScope();
}

void ScopeGeneration::visit(FunctionTypeDecl* ftdecl) {
    if (_nextMethodDecl != ftdecl && !ftdecl->getTypeArgs().empty()) {
        _ctx->reporter().error(*ftdecl, "Function type cannot be declared generic in this context");
    }

    pushScope(ftdecl);

    generateTypeParametersSymbols(ftdecl->getTypeArgs(), false);

    for (TypeExpression* arg : ftdecl->getArgTypes()) {
        arg->onVisit(this);
    }
    ftdecl->getRetType()->onVisit(this);
    ftdecl->getClassEquivalent()->onVisit(this);

    popScope();
}

void ScopeGeneration::visit(TypeConstructorCreation* tc) {
    pushScope(tc);

    TypeExpression* expr = tc->getArgs();
    std::vector<TypeExpression*> args;

    if (TypeTuple* ttuple = getIfNodeOfType<TypeTuple>(expr, _ctx)) { // form is `[] => ...` or `[exp, exp] => ...`, ...
        args = ttuple->getExpressions();
    } else { // form is `exp => ...` or `[exp] => ...`
        args.push_back(expr);
    }

    generateTypeParametersSymbols(args, true);

    //SAVE_MEMBER_AND_SET(_nextTypeExpr)

    tc->getBody()->onVisit(this);

    popScope();
}

void ScopeGeneration::visit(TypeParameter* tparam) {
    TypeDecl* defaultType = ASTDefaultTypeFromKindCreator::createDefaultTypeFromKind(
                tparam->getKindNode(), tparam->getSpecified()->getValue(), _ctx);

    createProperType(tparam->getSpecified(), defaultType);
}

void ScopeGeneration::visit(Block* block) {
    pushScope(block);

    ASTImplicitVisitor::visit(block);

    popScope();
}

void ScopeGeneration::visit(FunctionCreation* func) {
    pushScope(func);

    if (func->getTypeArgs()) {
        generateTypeParametersSymbols(func->getTypeArgs()->getExpressions(), false);
    }

    Expression* expr = func->getArgs();
    std::vector<Expression*> args;

    if (Tuple* tuple = getIfNodeOfType<Tuple>(expr, _ctx)) { // form is `() => ...` or `(exp, exp) => ...`, ...
        args = tuple->getExpressions();
    } else { // form is `exp => ...` or `(exp) => ...`
        args.push_back(expr);
    }

    bool wasModified = false;

    for (Expression*& expr : args) {
        if (Identifier* ident = getIfNodeOfType<Identifier>(expr, _ctx)) { // arg of the form `x`
            expr = _mngr.New<TypeSpecifier>(ident, _mngr.New<TypeToBeInferred>());
            expr->setPos(*ident);
            wasModified = true;
        }

        if(isNodeOfType<TypeSpecifier>(expr, _ctx)) { // arg of the form `x: type`
            // The var is going to be created by the TypeSpecifier Node
            expr->onVisit(this);
        } else {
            _ctx->reporter().error(*expr, "Argument should be an identifier");
        }
    }

    if (wasModified) {
        // update the function creation node
        common::Positionnable funcPos = *func;
        sym::Scoped funcScope = *func;
        Annotable funcAnnot = *func;

        Tuple* newArgs = _mngr.New<Tuple>(args);
        newArgs->setPos(*func->getArgs());
        *func = FunctionCreation(func->getName(), func->getTypeArgs(), newArgs, func->getBody(), func->getReturnType());

        *(static_cast<Annotable*>(func)) = funcAnnot;
        *(static_cast<sym::Scoped*>(func)) = funcScope;
        *(static_cast<common::Positionnable*>(func)) = funcPos;
    }

    if (TypeExpression* retType = func->getReturnType()) {
        retType->onVisit(this);
    }

    func->getBody()->onVisit(this);

    popScope();
}

void ScopeGeneration::visit(TypeSpecifier* tps) {
    tps->getTypeNode()->onVisit(this);
    createVar(tps->getSpecified());
}

void ScopeGeneration::createVar(Identifier* id) {
    std::string symName = id->getValue();
    sym::VariableSymbol* arg = _mngr.New<sym::VariableSymbol>(symName, symName);
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

void ScopeGeneration::generateTypeParametersSymbols(const std::vector<TypeExpression*>& typeParams, bool allowVarianceAnnotations) {
    for (TypeExpression* typeParam : typeParams) {
        if (TypeIdentifier* tident = getIfNodeOfType<TypeIdentifier>(typeParam, _ctx)) { // arg of the form `T`
            createProperType(tident, ASTDefaultTypeFromKindCreator::createDefaultTypeFromKind(
                                 _mngr.New<ProperTypeKindSpecifier>(), static_cast<TypeIdentifier*>(typeParam)->getValue(), _ctx));
        } else if(TypeParameter* tparam = getIfNodeOfType<TypeParameter>(typeParam, _ctx)) { // arg of the form `T: kind`
            // The type var is already going to be created by the TypeParameter Node
            typeParam->onVisit(this);

            if (!allowVarianceAnnotations && tparam->getVarianceType() != common::VAR_T_NONE) {
                _ctx->reporter().error(*typeParam, "Variance annotation are not allowed in this context");
            }
        } else {
            _ctx->reporter().error(*typeParam, "Type argument should be an identifier");
        }
    }
}

template<typename T, typename U>
T* ScopeGeneration::createSymbol(U* node) {
    std::string symName = node->getName()->getValue();
    T* sym = _mngr.New<T>(symName, absoluteName(symName));
    initCreated(node, sym);
    return sym;
}

sym::DefinitionSymbol* ScopeGeneration::createSymbol(DefineDecl* node, TypeExpression* currentThis) {
    std::string symName = node->getName()->getValue();
    sym::DefinitionSymbol* sym = _mngr.New<sym::DefinitionSymbol>(symName, absoluteName(symName), node, currentThis);
    initCreated(node, sym);
    return sym;
}

sym::TypeSymbol* ScopeGeneration::createSymbol(TypeDecl* node) {
    std::string symName = node->getName()->getValue();
    sym::TypeSymbol* sym = _mngr.New<sym::TypeSymbol>(symName, absoluteName(symName), node);
    initCreated(node, sym);
    return sym;
}

template<typename T, typename S>
void ScopeGeneration::initCreated(T* id, S* s) {
    s->setPos(*id);
    id->setSymbol(s);
    tryAddSymbol(s);
}

void ScopeGeneration::tryAddSymbol(sym::Symbol* sym) {
    if (sym::Symbol* oldSymbol = _curScope->addSymbol(sym)) {
        _ctx->reporter().error(*sym, std::string("Multiple definitions of symbol '") +
                                     sym->getName() + "' were found.");
        _ctx->reporter().info(*oldSymbol, "First instance here");
    }
}

void ScopeGeneration::pushPathPart(const std::string& name, bool becomesInvalid) {
    _symbolPath.push_back(name);
    if (becomesInvalid) {
        _invalidFrom = std::min(_invalidFrom, _symbolPath.size());
    }
}

bool ScopeGeneration::isValidAbsolutePath() const {
    return _symbolPath.size() < _invalidFrom;
}

std::string ScopeGeneration::absoluteName(const std::string& symName) {
    if (isValidAbsolutePath()) {
        return utils::join(_symbolPath, ".") + "." + symName;
    } else {
        return "";
    }
}

void ScopeGeneration::popPathPart() {
    _symbolPath.pop_back();
    if (isValidAbsolutePath()) {
        _invalidFrom = size_t_max;
    }
}

void ScopeGeneration::reportInvalidExternUsage(const common::Positionnable& pos) const {
    _ctx->reporter().error(pos, "extern can only be used for module-level declarations or inside classes that are declared at module-level");
}

// TYPE DEPENDENCY FIXATION


TypeDependencyFixation::TypeDependencyFixation(CompCtx_Ptr& ctx) : ASTImplicitVisitor(ctx) {

}

TypeDependencyFixation::~TypeDependencyFixation() {

}

void TypeDependencyFixation::visit(ClassDecl* clss) {
    clss->setParameters(_parameters);

    ASTImplicitVisitor::visit(clss);

#ifdef DEBUG_DEPENDENCIES
    debugDumpDependencies(clss);
#endif
}

void TypeDependencyFixation::visit(FunctionTypeDecl* ftdecl) {
    size_t pushed = pushTypeParameters(ftdecl->getTypeArgs());

    for (TypeExpression* arg : ftdecl->getArgTypes()) {
        arg->onVisit(this);
    }
    ftdecl->getRetType()->onVisit(this);
    ftdecl->getClassEquivalent()->onVisit(this);

    popTypeParameters(pushed);
}

void TypeDependencyFixation::visit(TypeConstructorCreation* tc) {
    tc->setParameters(_parameters);

    TypeExpression* expr = tc->getArgs();
    std::vector<TypeExpression*> args;

    if (TypeTuple* ttuple = getIfNodeOfType<TypeTuple>(expr, _ctx)) { // form is `[] => ...` or `[exp, exp] => ...`, ...
        args = ttuple->getExpressions();
    } else { // form is `exp => ...` or `[exp] => ...`
        args.push_back(expr);
    }

    size_t pushed = pushTypeParameters(args);

    tc->getBody()->onVisit(this);

#ifdef DEBUG_DEPENDENCIES
    debugDumpDependencies(tc);
#endif

    popTypeParameters(pushed);
}

void TypeDependencyFixation::visit(FunctionCreation* func) {
    func->setParameters(_parameters);

    size_t pushed = 0;

    if (func->getTypeArgs()) {
        func->getTypeArgs()->onVisit(this);
        pushed = pushTypeParameters(func->getTypeArgs()->getExpressions());
    }

    func->getArgs()->onVisit(this);

    if (TypeExpression* retType = func->getReturnType()) {
        retType->onVisit(this);
    }

    func->getBody()->onVisit(this);

#ifdef DEBUG_DEPENDENCIES
    debugDumpDependencies(func);
#endif

    popTypeParameters(pushed);
}

size_t TypeDependencyFixation::pushTypeParameters(const std::vector<TypeExpression*>& typeParams) {
    size_t pushed = 0;
    for (TypeExpression* typeParam : typeParams) {
        TypeIdentifier* id;
        common::VARIANCE_TYPE vt = common::VAR_T_NONE;

        if (TypeIdentifier* tident = getIfNodeOfType<TypeIdentifier>(typeParam, _ctx)) { // arg of the form `T`
            id = tident;
        } else if(TypeParameter* tparam = getIfNodeOfType<TypeParameter>(typeParam, _ctx)) { // arg of the form `x: kind`
            vt = tparam->getVarianceType();
            id = tparam->getSpecified();
        } else {
            break; // Error already reported in scope generation
                   //  => compiler will stop before type dependency is even used
        }

        if (sym::TypeSymbol* tpsym = sym::getIfSymbolOfType<sym::TypeSymbol>(id->getSymbol())) {
            _parameters.push_back(Parameter(vt, tpsym));
            ++pushed;
        } else {
            _ctx->reporter().fatal(*typeParam, "Is supposed to be a TypeSymbol");
        }
    }
    return pushed;
}

void TypeDependencyFixation::popTypeParameters(size_t pushed) {
    _parameters.resize(_parameters.size() - pushed);
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

SymbolAssignation::SymbolAssignation(CompCtx_Ptr& ctx) : ASTImplicitVisitor(ctx), _curScope(nullptr) {
}

SymbolAssignation::~SymbolAssignation() {

}

void SymbolAssignation::visit(ModuleDecl* mod) {
    SAVE_SCOPE(mod->getSymbol())

    buildUsingsFromPaths(mod);

    ASTImplicitVisitor::visit(mod);

    RESTORE_SCOPE
}

void SymbolAssignation::visit(TypeDecl* tdecl) {
    SAVE_SCOPE(tdecl->getSymbol())

    ASTImplicitVisitor::visit(tdecl);

    RESTORE_SCOPE
}

void SymbolAssignation::visit(ClassDecl* clss) {
    visitParent(clss);

    SAVE_SCOPE(clss)

    for (TypeDecl* tdecl : clss->getTypeDecls()) {
        tdecl->onVisit(this);
    }
    for (TypeSpecifier* field : clss->getFields()) {
        field->onVisit(this);
    }
    for (DefineDecl* def : clss->getDefs()) {
        def->onVisit(this);
    }

    RESTORE_SCOPE
}

void SymbolAssignation::visit(DefineDecl* def) {
    SAVE_SCOPE(def->getSymbol())

    ASTImplicitVisitor::visit(def);

    RESTORE_SCOPE
}

void SymbolAssignation::visit(FunctionTypeDecl* ftdecl) {
    SAVE_SCOPE(ftdecl)

    ASTImplicitVisitor::visit(ftdecl);

    RESTORE_SCOPE
}

void SymbolAssignation::visit(TypeMemberAccess* tmac) {
    assignMemberAccess(tmac);
}

void SymbolAssignation::visit(TypeConstructorCreation* tc) {
    SAVE_SCOPE(tc)

    ASTImplicitVisitor::visit(tc);

    RESTORE_SCOPE
}

void SymbolAssignation::visit(TypeIdentifier* id) {
    assignIdentifier(id);
}

void SymbolAssignation::visit(TypeSpecifier* tps) {
    tps->getTypeNode()->onVisit(this);
}

void SymbolAssignation::visit(Block* block) {
    SAVE_SCOPE(block)

    buildUsingsFromPaths(block);

    ASTImplicitVisitor::visit(block);

    RESTORE_SCOPE
}

void SymbolAssignation::visit(MemberAccess* mac) {
    assignMemberAccess(mac);
}

void SymbolAssignation::visit(FunctionCreation* func) {
    SAVE_SCOPE(func)

    ASTImplicitVisitor::visit(func);

    RESTORE_SCOPE
}

void SymbolAssignation::visit(FunctionCall* call) {
    ASTImplicitVisitor::visit(call);

    if (sym::Symbol* s = ASTSymbolExtractor::extractSymbol(call->getCallee(), _ctx)) {
        if (s->getSymbolType() == sym::SYM_TPE) {
            TypeExpression* expr = ASTExpr2TypeExpr::convert(call->getCallee(), _ctx);
            if (!expr) {
                _ctx->reporter().fatal(*call->getCallee(), "Expression cannot be converted to a type expression");
                return;
            }

            if (call->getTypeArgsTuple()) {
                expr = _mngr.New<TypeConstructorCall>(expr, call->getTypeArgsTuple());
                common::Positionnable pos = *call->getCallee();
                pos.setEndPos(call->getTypeArgsTuple()->getEndPosition());
                expr->setPos(pos);
            }

            Instantiation* inst = _mngr.New<Instantiation>(expr);
            inst->setPos(*call->getCallee());

            common::Positionnable callPos = *call;
            *call = FunctionCall(inst, nullptr, call->getArgsTuple());
            call->setPos(callPos);
        }
    }
}

void SymbolAssignation::visit(Identifier* id) {
    assignIdentifier(id);
}

void SymbolAssignation::buildUsingsFromPaths(const CanUseModules* canUseModules) {
    _curScope->buildUsingsFromPaths(_ctx, *canUseModules);
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
        if (sym::ModuleSymbol* modsym = sym::getIfSymbolOfType<sym::ModuleSymbol>(sym)) {
            assignFromStaticScope(mac, modsym, "module " + sym->getName());
            return;
        }
    }

    if (type::Type* t = ASTTypeCreator::createType(mac->getAccessed(), _ctx)) {
        assignFromTypeScope(mac, t);
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
void SymbolAssignation::assignFromTypeScope(T* mac, type::Type* t) {
    // TODO: change this when other constructs allow having type members
    if (type::ProperType* pt = type::getIf<type::ProperType>(t->applyTCCallsOnly(_ctx))) {
        if (!pt->getClass()->getScope()->assignSymbolic(*mac, mac->getMember()->getValue())) { // update member access symbolic for potential later use
            _ctx->reporter().error(*mac->getMember(), "No member named " + mac->getMember()->getValue() + " in class " + pt->getClass()->getName());
        }
    } else {
        _ctx->reporter().error(*mac->getAccessed(), "Type " + t->toString() + " cannot have any members");
    }
}

bool SymbolAssignation::visitParent(ClassDecl* clss) {
    if (_temporarilyVisitedTypes.find(clss) != _temporarilyVisitedTypes.end()) {
        _ctx->reporter().error(*clss, "Class " + clss->getName() + " is part of an inheritance cycle");
        return false;
    }

    if (_visitedTypes.find(clss) == _visitedTypes.end()) { // if unmarked
        auto it = _temporarilyVisitedTypes.insert(clss).first; // mark temporarily

        if (clss->getParent()) {
            SAVE_SCOPE(clss)

            clss->getParent()->onVisit(this);
            if (type::Type* parentType = ASTTypeCreator::createType(clss->getParent(), _ctx)) {
                if (type::ProperType* parent = type::getIf<type::ProperType>(parentType->apply(_ctx))) {
                    ClassDecl* parentClass = parent->getClass();

                    if (visitParent(parentClass)) {
                        _curScope->copySymbolsFrom(parentClass->getScope(), parent->getEnvironment(), sym::Scope::ExcludeConstructors);
                        clss->addSuperType(parentClass, parent->getEnvironment());
                    }
                }
            }

            RESTORE_SCOPE
        }

        clss->addSpecialSuperType(clss, ASTTypeCreator::buildEnvironmentFromTypeParametrizable(clss));

        _temporarilyVisitedTypes.erase(it); // unmark temporarily
        _visitedTypes.insert(clss); // mark permantently
    }

    return true;
}

}

}
