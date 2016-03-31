//
//  AST2BAST.cpp
//  SFSL
//
//  Created by Romain Beguet on on 28.03.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#include "AST2BAST.h"
#include "../../Frontend/AST/Visitors/ASTSymbolExtractor.h"
#include "../../Frontend/AST/Visitors/ASTTypeIdentifier.h"

namespace sfsl {

namespace bast {

// AST2BAST

AST2BAST::AST2BAST(CompCtx_Ptr& ctx) : ASTExplicitVisitor(ctx), _freshId(0), _rep(ctx->reporter()), _created(nullptr) {

}

AST2BAST::~AST2BAST() {

}

void AST2BAST::visit(ast::ASTNode*) {

}

void AST2BAST::visit(ast::Program* prog) {
    for (ast::ModuleDecl* module : prog->getModules()) {
        module->onVisit(this);
    }
    make<Program>(_visibleDefs, _hiddenDefs);
}

void AST2BAST::visit(ast::ModuleDecl* module) {
    for (ast::ModuleDecl* mod : module->getSubModules()) {
        mod->onVisit(this);
    }

    for (ast::TypeDecl* type : module->getTypes()) {
        type->onVisit(this);
    }

    for (ast::DefineDecl* decl : module->getDeclarations()) {
        decl->onVisit(this);
    }
}

void AST2BAST::visit(ast::TypeDecl* tdecl) {
    addDefinitionToProgram<GlobalDef>(tdecl->getSymbol(), transform(tdecl->getExpression()));
    makeUnit();
}

void AST2BAST::visit(ast::ClassDecl* clss) {
    ast::ClassUserData* classUD = clss->getUserdata<ast::ClassUserData>();
    DefIdentifier* parentId = nullptr;

    if (!alreadyTransformed(classUD)) {
        if (clss->getParent()) {
            std::string name = freshName("parent");
            _hiddenDefs.push_back(make<GlobalDef>(name, transform(clss->getParent())));
            parentId = make<DefIdentifier>(name);
        }

        for (ast::TypeDecl* tdecl: clss->getTypeDecls()) {
            tdecl->onVisit(this);
        }

        for (ast::DefineDecl* decl : clss->getDefs()) {
            decl->onVisit(this);
        }

        std::vector<DefIdentifier*> methods;

        for (sym::DefinitionSymbol* def : classUD->getDefs()) {
            methods.push_back(make<DefIdentifier>(getDefId(def)));
        }

        addDefinitionToProgram<ClassDef>(clss, classUD->getAttrCount(), parentId, methods);
    }

    make<DefIdentifier>(getDefId(clss));
}

void AST2BAST::visit(ast::DefineDecl* decl) {
    if (decl->isExtern() || decl->isAbstract()) {
        return;
    }

    addDefinitionToProgram<GlobalDef>(decl->getSymbol(), transform(decl->getValue()));
    makeUnit();
}

void AST2BAST::visit(ast::ProperTypeKindSpecifier* ptks) {
    makeBad();
}

void AST2BAST::visit(ast::TypeConstructorKindSpecifier* tcks) {
    makeBad();
}

void AST2BAST::visit(ast::FunctionTypeDecl* ftdecl) {

}

void AST2BAST::visit(ast::TypeMemberAccess* tdot) {
    visitSymbolic(tdot);
}

void AST2BAST::visit(ast::TypeTuple* ttuple) {
    makeBad();
}

void AST2BAST::visit(ast::TypeConstructorCreation* typeconstructor) {
    typeconstructor->getBody()->onVisit(this);
}

void AST2BAST::visit(ast::TypeConstructorCall* tcall) {
    tcall->getCallee()->onVisit(this);
    for (ast::TypeExpression* arg : tcall->getArgs()) {
        arg->onVisit(this);
    }
}

void AST2BAST::visit(ast::TypeIdentifier* tident) {
    visitSymbolic(tident);
}

void AST2BAST::visit(ast::TypeToBeInferred* tbi) {
    makeBad();
}

void AST2BAST::visit(ast::KindSpecifier* ks) {
    makeBad();
}

void AST2BAST::visit(ast::ExpressionStatement* exp) {
    transform(exp->getExpression());
}

void AST2BAST::visit(ast::AssignmentExpression* aex) {
    if (ast::isNodeOfType<ast::TypeSpecifier>(aex->getLhs(), _ctx)) {
        ast::TypeSpecifier* tps = static_cast<ast::TypeSpecifier*>(aex->getLhs());
        assignIdentifier(tps->getSpecified(), aex->getRhs());
    }
    else if (ast::isNodeOfType<ast::MemberAccess>(aex->getLhs(), _ctx)) {
        ast::MemberAccess* mac = static_cast<ast::MemberAccess*>(aex->getLhs());
        if (mac->getSymbol()->getSymbolType() == sym::SYM_VAR) {
            sym::VariableSymbol* var = static_cast<sym::VariableSymbol*>(mac->getSymbol());
            make<FieldAssignmentExpression>(transform(mac->getAccessed()), getVarLoc(var), transform(aex->getRhs()));
        } else {
            _rep.fatal(*mac, "Shouldn't be able to assign non var member");
        }
    }
    else if (ast::isNodeOfType<ast::IfExpression>(aex->getLhs(), _ctx)) {
        // TODO
    }
    else if (ast::isNodeOfType<ast::Tuple>(aex->getLhs(), _ctx)) {
        // TODO
    }
    else if (ast::isNodeOfType<ast::Identifier>(aex->getLhs(), _ctx)) {
        assignIdentifier(static_cast<ast::Identifier*>(aex->getLhs()), aex->getRhs());
    } else {
        _rep.fatal(*aex, "Is not allowed");
    }
}

void AST2BAST::visit(ast::TypeSpecifier* tps) {
    makeUnit();
}

void AST2BAST::visit(ast::Block* block) {
    std::vector<Expression*> bexprs;
    for (ast::Expression* expr : block->getStatements()) {
        bexprs.push_back(transform(expr));
    }
    make<Block>(bexprs);
}

void AST2BAST::visit(ast::IfExpression* ifexpr) {
    Expression* cond = transform(ifexpr->getCondition());
    Expression* then = transform(ifexpr->getThen());
    Expression* els = ifexpr->getElse()
            ? transform(ifexpr->getElse())
            : make<Block>(std::vector<Expression*>());

    make<IfExpression>(cond, then, els);
}

void AST2BAST::visit(ast::MemberAccess* dot) {
    if (dot->getSymbol()->getSymbolType() == sym::SYM_VAR) {
        size_t fieldLoc = getVarLoc(static_cast<sym::VariableSymbol*>(dot->getSymbol()));
        make<FieldAccess>(transform(dot->getAccessed()), fieldLoc);
    } else {
        visitSymbolic(dot);
    }
}

void AST2BAST::visit(ast::Tuple* tuple) {
    makeUnit();
}

void AST2BAST::visit(ast::FunctionCreation* func) {
    ast::FuncUserData* fUD = func->getUserdata<ast::FuncUserData>();

    if (type::ProperType* pt = type::getIf<type::ProperType>(func->type())) {
        transform(pt->getClass());
        // TODO: Call the constructor of the function's class
        make<Instantiation>(make<DefIdentifier>(getDefId(pt->getClass())));
    } else {
        addDefinitionToProgram<MethodDef>(func, fUD->getVarCount(), transform(func->getBody()));
        make<DefIdentifier>(getDefId(func));
    }
}

void AST2BAST::visit(ast::FunctionCall* call) {
    if (call->getCallee()->type()->getTypeKind() == type::TYPE_METHOD) {
        sym::DefinitionSymbol* def = static_cast<sym::DefinitionSymbol*>(ast::ASTSymbolExtractor::extractSymbol(call->getCallee(), _ctx));
        size_t virtualLoc = def->getUserdata<ast::VirtualDefUserData>()->getVirtualLocation();

        Expression* callee;
        std::vector<Expression*> args;

        if (ast::isNodeOfType<ast::MemberAccess>(call->getCallee(), _ctx)) {
            ast::MemberAccess* dot = static_cast<ast::MemberAccess*>(call->getCallee());
            callee = transform(dot->getAccessed());
        } else if (ast::isNodeOfType<ast::Identifier>(call->getCallee(), _ctx)) { // implicit this
            callee = make<VarIdentifier>(0);
        }

        for (ast::Expression* expr : call->getArgs()) {
            args.push_back(transform(expr));
        }

        make<MethodCall>(callee, virtualLoc, args);
    }
}

void AST2BAST::visit(ast::Instantiation* inst) {
    std::string name = freshName("inst");
    _hiddenDefs.push_back(make<GlobalDef>(name, transform(inst->getInstantiatedExpression())));
    make<Instantiation>(make<DefIdentifier>(name));
}

void AST2BAST::visit(ast::Identifier* ident) {
    visitSymbolic(ident);
}

void AST2BAST::visit(ast::This*) {
    make<VarIdentifier>(0);
}

void AST2BAST::visit(ast::BoolLitteral* boollit) {
    make<BoolLitteral>(boollit->getValue());
}

void AST2BAST::visit(ast::IntLitteral* intlit) {
    make<IntLitteral>(intlit->getValue());
}

void AST2BAST::visit(ast::RealLitteral* reallit) {
    make<RealLitteral>(reallit->getValue());
}

void AST2BAST::visit(ast::StringLitteral* strlit) {
    make<StringLitteral>(strlit->getValue());
}

Program* AST2BAST::transform(ast::Program* node) {
    node->onVisit(this);
    return static_cast<Program*>(_created);
}

template<typename T>
void AST2BAST::visitSymbolic(T* symbolic) {
    if (symbolic->getSymbolCount() != 1) {
        _rep.error(*symbolic, "Symbolic refers to several symbols");
        makeBad();
    } else {
        switch (symbolic->getSymbol()->getSymbolType()) {
        case sym::SYM_VAR: {
            sym::VariableSymbol* var = static_cast<sym::VariableSymbol*>(symbolic->getSymbol());
            if (isVariableAttribute(var)) {
                make<FieldAccess>(make<VarIdentifier>(0), getVarLoc(var));
            } else {
                make<VarIdentifier>(getVarLoc(var));
            }
            break;
        }

        case sym::SYM_DEF:
            make<DefIdentifier>(getDefId(static_cast<sym::DefinitionSymbol*>(symbolic->getSymbol())));
            break;

        case sym::SYM_TPE: {
            make<DefIdentifier>(getDefId(static_cast<sym::TypeSymbol*>(symbolic->getSymbol())));
            break;
        }

        default:
            _rep.fatal(*symbolic, "Cannot generate expression for this kind of symbolic");
            break;
        }
    }
}

void AST2BAST::assignIdentifier(ast::Identifier* ident, ast::Expression* val) {
    if (ident->getSymbolCount() != 1) {
        _rep.error(*ident, "Identifier refers to several symbols");
        makeBad();
    } else if (ident->getSymbol()->getSymbolType() == sym::SYM_VAR) {
        sym::VariableSymbol* var = static_cast<sym::VariableSymbol*>(ident->getSymbol());
        if (isVariableAttribute(var)) {
            make<FieldAssignmentExpression>(make<VarIdentifier>(0), getVarLoc(var), transform(val));
        } else {
            make<VarAssignmentExpression>(getVarLoc(var), transform(val));
        }
    } else {
        _rep.fatal(*ident, "Cannot assign identifier to this kind of symbol");
    }
}

const std::string& AST2BAST::getDefId(ast::FunctionCreation* func) {
    return func->getUserdata<ast::DefUserData>()->getDefId();
}

const std::string& AST2BAST::getDefId(ast::ClassDecl* clss) {
    return clss->getUserdata<ast::ClassUserData>()->getDefId();
}

const std::string& AST2BAST::getDefId(sym::DefinitionSymbol* def) {
    ast::DefUserData* defUD = def->getUserdata<ast::DefUserData>();
    return defUD ? defUD->getDefId() : def->getAbsoluteName();
}

const std::string& AST2BAST::getDefId(sym::TypeSymbol* tpe) {
    ast::DefUserData* defUD = tpe->getUserdata<ast::DefUserData>();
    return defUD ? defUD->getDefId() : tpe->getAbsoluteName();
}

template<typename T>
bool AST2BAST::isVisibleDef(T* def) const {
    ast::DefUserData* defUD = def->template getUserdata<ast::DefUserData>();
    return defUD->isVisible();
}

template<typename BAST_NODE, typename DEF, typename... BAST_ARGS_REST>
void AST2BAST::addDefinitionToProgram(DEF* def, BAST_ARGS_REST... args) {
    BAST_NODE* node = make<BAST_NODE>(getDefId(def), std::forward<BAST_ARGS_REST>(args)...);
    if (isVisibleDef(def)) {
        _visibleDefs.push_back(node);
    } else {
        _hiddenDefs.push_back(node);
    }
}

size_t AST2BAST::getVarLoc(sym::VariableSymbol* var) {
    return var->getUserdata<ast::VarUserData>()->getVarLoc();
}

bool AST2BAST::isVariableAttribute(sym::VariableSymbol* var) {
    return var->getUserdata<ast::VarUserData>()->isAttribute();
}

Expression* AST2BAST::makeUnit() {
    return make<UnitLitteral>();
}

template<typename T, typename... Args>
T* AST2BAST::make(Args... args) {
    T* toRet = _mngr.New<T>(std::forward<Args>(args)...);
    _created = toRet;
    return toRet;
}

BASTNode* AST2BAST::makeBad() {
    return _created = nullptr;
}

Expression* AST2BAST::transform(ast::Expression* node) {
    node->onVisit(this);
    return static_cast<Expression*>(_created);
}

bool AST2BAST::alreadyTransformed(ast::DefUserData* defUD) {
    for (Definition* def : _visibleDefs) {
        if (def->getName() == defUD->getDefId()) {
            return true;
        }
    }
    return false;
}

std::string AST2BAST::freshName(const std::string& prefix) {
    return prefix + "$$" + std::to_string(_freshId++);
}

// BAST SIMPLIFIER

BASTSimplifier::BASTSimplifier(CompCtx_Ptr& ctx) : BASTImplicitVisitor(ctx) {

}

BASTSimplifier::~BASTSimplifier() {

}

void BASTSimplifier::visit(Program* prog) {
    Analyser als(_ctx);
    prog->onVisit(&als);

    HiddenToAnyRenamer htar(_ctx, als.getHiddenToAnyMappings());
    prog->onVisit(&htar);

    VisibleToHiddenRenamer vthr(_ctx, als.getVisibleToHiddenMappings());
    prog->onVisit(&vthr);
}

// ANALYSER

BASTSimplifier::Analyser::Analyser(CompCtx_Ptr& ctx)
        : BASTExplicitVisitor(ctx), _processingVisibleNames(false), _name(nullptr) {

}

BASTSimplifier::Analyser::~Analyser() {

}

void BASTSimplifier::Analyser::visit(Program* prog) {
    _processingVisibleNames = true;
    for (Definition* def : prog->getVisibleDefinitions()) {
        _visibleNames.insert(def->getName());
        def->onVisit(this);
    }

    _processingVisibleNames = false;
    for (Definition* def : prog->getHiddenDefinitions()) {
        def->onVisit(this);
    }

    for (auto& pair : _hiddenToAnyNameMappings) {
        pair.second = findSubstitution(pair.second);
    }
}

void BASTSimplifier::Analyser::visit(GlobalDef* global) {
    _name = nullptr;
    global->getBody()->onVisit(this);
    if (_name) {
        if (_processingVisibleNames) {
            if (isHiddenName(*_name)) {
                _visibleToHiddenNameMappings[*_name] = global->getName();
            }
        } else {
            _hiddenToAnyNameMappings[global->getName()] = *_name;
        }
    }
}

void BASTSimplifier::Analyser::visit(DefIdentifier* defid) {
    _name = &defid->getValue();
}

const std::map<std::string, std::string>& BASTSimplifier::Analyser::getHiddenToAnyMappings() const {
    return _hiddenToAnyNameMappings;
}

const std::map<std::string, std::string>& BASTSimplifier::Analyser::getVisibleToHiddenMappings() const {
    return _visibleToHiddenNameMappings;
}

bool BASTSimplifier::Analyser::isHiddenName(const std::string& name) const {
    return _visibleNames.find(name) == _visibleNames.end();
}

std::string BASTSimplifier::Analyser::findSubstitution(std::string name) const {
    while (true) {
        auto it1 = _visibleToHiddenNameMappings.find(name);
        auto it2 = _hiddenToAnyNameMappings.end();

        if (it1 != _visibleToHiddenNameMappings.end()) {
            it2 = _hiddenToAnyNameMappings.find(name);
        }

        if (it1 != _visibleToHiddenNameMappings.end()) {
            name = it1->second;
        } else if (it2 != _hiddenToAnyNameMappings.end()) {
            name = it2->second;
        } else {
            break;
        }
    }
    return name;
}

// HIDDEN TO ANY RENAMER

BASTSimplifier::HiddenToAnyRenamer::HiddenToAnyRenamer(CompCtx_Ptr& ctx, const std::map<std::string, std::string>& map)
    : BASTImplicitVisitor(ctx), _map(map) {

}

BASTSimplifier::HiddenToAnyRenamer::~HiddenToAnyRenamer() {

}

void BASTSimplifier::HiddenToAnyRenamer::visit(Program* prog) {
    BASTImplicitVisitor::visit(prog);

    std::vector<Definition*> newHiddenDefinitions;

    for (Definition* hidden : prog->getHiddenDefinitions()) {
        auto it = _map.find(hidden->getName());
        if (it == _map.end()) {
            newHiddenDefinitions.push_back(hidden);
        }
    }

    *prog = Program(prog->getVisibleDefinitions(), newHiddenDefinitions);
}

void BASTSimplifier::HiddenToAnyRenamer::visit(DefIdentifier* defid) {
    auto it = _map.find(defid->getValue());
    if (it != _map.end()) {
        *defid = DefIdentifier(it->second);
    }
}

// VISIBLE TO HIDDEN RENAMER

BASTSimplifier::VisibleToHiddenRenamer::VisibleToHiddenRenamer(CompCtx_Ptr& ctx, const std::map<std::string, std::string>& map)
    : BASTImplicitVisitor(ctx), _map(map) {

}

BASTSimplifier::VisibleToHiddenRenamer::~VisibleToHiddenRenamer() {

}

void BASTSimplifier::VisibleToHiddenRenamer::visit(Program* prog) {
    std::vector<Definition*> newVisibleDefinitions;
    std::vector<Definition*> newHiddenDefinitions;

    for (Definition* visible : prog->getVisibleDefinitions()) {
        _nextExpr = nullptr;
        _toDelete = false;
        visible->onVisit(this);

        if (!_toDelete) {
            newVisibleDefinitions.push_back(visible);
        }
    }
    for (Definition* hidden : prog->getHiddenDefinitions()) {
        auto it = _map.find(hidden->getName());
        hidden->onVisit(this);
        if (it != _map.end()) {
            newVisibleDefinitions.push_back(hidden);
        } else {
            newHiddenDefinitions.push_back(hidden);
        }
    }

    *prog = Program(newVisibleDefinitions, newHiddenDefinitions);
}

void BASTSimplifier::VisibleToHiddenRenamer::visit(MethodDef* meth) {
    auto it = _map.find(meth->getName());
    if (it != _map.end()) {
        *meth = MethodDef(it->second, meth->getVarCount(), meth->getMethodBody());
    }
    BASTImplicitVisitor::visit(meth);
}

void BASTSimplifier::VisibleToHiddenRenamer::visit(ClassDef* clss) {
    auto it = _map.find(clss->getName());
    if (it != _map.end()) {
        *clss = ClassDef(it->second, clss->getFieldCount(), clss->getParent(), clss->getMethods());
    }
    BASTImplicitVisitor::visit(clss);
}

void BASTSimplifier::VisibleToHiddenRenamer::visit(GlobalDef* global) {
    auto it = _map.find(global->getName());
    if (it != _map.end()) {
        *global = GlobalDef(it->second, global->getBody());
    }
    _nextExpr = global->getBody();
    BASTImplicitVisitor::visit(global);
}

void BASTSimplifier::VisibleToHiddenRenamer::visit(DefIdentifier* defid) {
    auto it = _map.find(defid->getValue());
    if (it != _map.end()) {
        if (_nextExpr == defid) {
            _toDelete = true;
        } else {
            *defid = DefIdentifier(it->second);
        }
    }
}

}

}
