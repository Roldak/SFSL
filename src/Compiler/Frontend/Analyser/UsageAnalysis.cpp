//
//  UsageAnalysis.cpp
//  SFSL
//
//  Created by Romain Beguet on 01.08.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#include "UsageAnalysis.h"
#include "../Symbols/Symbols.h"
#include "../AST/Visitors/ASTTypeCreator.h"
#include "../AST/Visitors/ASTAssignmentChecker.h"

namespace sfsl {

namespace ast {

// CAPTURES USER DATA

struct CapturesUserData : common::MemoryManageable {
    CapturesUserData(const std::map<sym::VariableSymbol*, std::vector<Identifier*>>& captures, ASTNode* unitDeclaredIn)
        : captures(captures), unitDeclaredIn(unitDeclaredIn) {}

    std::map<sym::VariableSymbol*, std::vector<Identifier*>> captures;
    ASTNode* unitDeclaredIn;
};

// USAGE ANALYSIS

class LocalUsageAnalysis : ASTExplicitVisitor {
public:

    LocalUsageAnalysis(CompCtx_Ptr& ctx, std::map<sym::VariableSymbol*, std::vector<Identifier*>>& undeclaredVars)
        : ASTExplicitVisitor(ctx), _undeclaredVars(undeclaredVars) {}

    virtual ~LocalUsageAnalysis() {}

    void analyse(FunctionCreation* func) {
        for (sym::VariableSymbol* var : ASTAssignmentChecker::getAssignedVars(func->getArgs(), _ctx)) {
            declare(var);
            init(var);
        }

        func->getBody()->onVisit(this);

        warnForUnusedVariable();
    }

    void analyse(DefineDecl* def) {
        if (def->getValue()) {
            def->getValue()->onVisit(this);
            warnForUnusedVariable();
        }
    }

protected:

    virtual void visit(ExpressionStatement* expr) override {
        expr->getExpression()->onVisit(this);
    }

    virtual void visit(AssignmentExpression* aex) override {
        aex->getRhs()->onVisit(this);

        std::vector<sym::VariableSymbol*> assignedVars = ASTAssignmentChecker::getAssignedVars(aex->getLhs(), _ctx);
        if (assignedVars.empty()) {
            _ctx->reporter().error(*aex->getLhs(), "This expression is not assignable");
        }

        for (sym::VariableSymbol* var : assignedVars) {
            var->unsetProperty(UsageProperty::USABLE);
        }

        aex->getLhs()->onVisit(this);

        for (sym::VariableSymbol* var : assignedVars) {
            var->setProperty(UsageProperty::USABLE);
            if (var->hasProperty(UsageProperty::INITIALIZED)) {
                var->setProperty(UsageProperty::MUTABLE);
            } else {
                init(var);
            }
        }
    }

    virtual void visit(TypeSpecifier* tps) override {
        if (sym::VariableSymbol* var = sym::getIfSymbolOfType<sym::VariableSymbol>(tps->getSpecified()->getSymbol())) {
            declare(var);
        }
    }

    virtual void visit(Block* block) override {
        for (Expression* expr : block->getStatements()) {
            expr->onVisit(this);
        }
    }

    virtual void visit(IfExpression* ifexpr) override {
        ifexpr->getCondition()->onVisit(this);
        SAVE_MEMBER(_initCurScope)

        _initCurScope.clear();

        ifexpr->getThen()->onVisit(this);
        std::vector<sym::VariableSymbol*> thenInits = std::move(_initCurScope);
        uninit(thenInits);

        _initCurScope.clear();

        if (ifexpr->getElse()) {
            ifexpr->getElse()->onVisit(this);
        }
        std::vector<sym::VariableSymbol*> elseInits = std::move(_initCurScope);
        uninit(elseInits);

        RESTORE_MEMBER(_initCurScope)

        for (sym::VariableSymbol* thenInit : thenInits) {
            for (sym::VariableSymbol* elseInit : elseInits) {
                if (thenInit == elseInit) {
                    init(thenInit);
                }
            }
        }
    }

    virtual void visit(MemberAccess* dot) override {
        dot->getAccessed()->onVisit(this);
    }

    virtual void visit(Tuple* tuple) override {
        for (Expression* expr : tuple->getExpressions()) {
            expr->onVisit(this);
        }
    }

    virtual void visit(FunctionCreation* func) override {
        useCapturedVarsOnInstantiation(func, *func);
    }

    virtual void visit(FunctionCall* call) override {
        call->getCallee()->onVisit(this);
        call->getArgsTuple()->onVisit(this);
    }

    virtual void visit(Instantiation* inst) override {
        TypeExpression* instantiatedExpr = inst->getInstantiatedExpression();

        while (true) {
            if (type::Type* tp = ASTTypeCreator::createType(instantiatedExpr, _ctx)) {
                if (type::ProperType* pt = type::getIf<type::ProperType>(tp->applyTCCallsOnly(_ctx))) {
                    useCapturedVarsOnInstantiation(pt->getClass(), *inst);
                    if ((instantiatedExpr = pt->getClass()->getParent())) {
                        continue;
                    }
                }
            }

            break;
        }
    }

    virtual void visit(Identifier* ident) override {
        if (sym::Symbol* s = ident->getSymbol()) {
            if (sym::VariableSymbol* var = sym::getIfSymbolOfType<sym::VariableSymbol>(s)) {
                use(var, *ident, ident);
            }
        }
    }

    void declare(sym::VariableSymbol* var) {
        var->setProperty(UsageProperty::DECLARED);
        _declaredVars.push_back(var);
        _undeclaredVars.erase(var);

        for (Identifier* ident : _locallyUndeclaredVars[var]) {
            _ctx->reporter().error(*ident, "Variable `" + var->getName() + "` is used or assigned before being declared");
        }
    }

    void init(sym::VariableSymbol* var) {
        var->setProperty(UsageProperty::INITIALIZED);
        _initCurScope.push_back(var);
    }

    void use(sym::VariableSymbol* var, const common::Positionnable& pos, Identifier* by = nullptr) {
        if (var->hasProperty(UsageProperty::USABLE)) {
            var->setProperty(UsageProperty::USED);
        }

        if (!var->hasProperty(UsageProperty::DECLARED) && by) {
            _undeclaredVars[var].push_back(by);
            _locallyUndeclaredVars[var].push_back(by);
        } else if (var->hasProperty(UsageProperty::USABLE) && var->hasProperty(UsageProperty::DECLARED) && !var->hasProperty(UsageProperty::INITIALIZED)) {
            _ctx->reporter().error(pos, "Variable `" + var->getName() +
                                   ( by ? "` is used here before being initialized" :
                                          "` is captured here before being initialized"));
        }
    }

    void uninit(const std::vector<sym::VariableSymbol*>& vars) {
        for (sym::VariableSymbol* var : vars) {
            var->unsetProperty(UsageProperty::INITIALIZED);
        }
    }

    void useCapturedVarsOnInstantiation(common::HasManageableUserdata* ud, const common::Positionnable& pos) {
        if (ud) {
            if (CapturesUserData* capturesData = ud->getUserdata<CapturesUserData>()) {
                for (const auto& capture : capturesData->captures) {
                    use(capture.first, pos);
                }
            }
        }
    }

    static bool isSupposedToBeUnused(const std::string& name) {
        const std::string unusedPrefix = "unused_";
        if (name.size() >= unusedPrefix.size()) {
            if (name.substr(0, unusedPrefix.size()) == unusedPrefix) {
                return true;
            }
        }
        return false;
    }

    void warnForUnusedVariable() {
        for (sym::VariableSymbol* var : _declaredVars) {
            if (!var->hasProperty(UsageProperty::USED) && !isSupposedToBeUnused(var->getName())) {
                _ctx->reporter().warning(*var, "Unused variable '" + var->getName() + "'");
            }
        }
    }

    std::map<sym::VariableSymbol*, std::vector<Identifier*>>& _undeclaredVars;
    std::map<sym::VariableSymbol*, std::vector<Identifier*>> _locallyUndeclaredVars;
    std::vector<sym::VariableSymbol*> _declaredVars;
    std::vector<sym::VariableSymbol*> _initCurScope;
};

// USAGE ANALYSIS

#define SAVE_UNDECLARED_VARS SAVE_MEMBER_AND_SET(_undeclaredVars, {})
#define SET_CAPTURES_AND_UPDATE_UNDECLARED_VARS(obj) {\
    if (_undeclaredVars.size() > 0) { \
        obj->setUserdata(_mngr.New<CapturesUserData>(_undeclaredVars, _currentUnit)); \
        _undeclaredVars.insert(OLD(_undeclaredVars).cbegin(), OLD(_undeclaredVars).cend()); \
    } else { \
        RESTORE_MEMBER(_undeclaredVars) \
    } \
}

UsageAnalysis::UsageAnalysis(CompCtx_Ptr& ctx)
    : ASTImplicitVisitor(ctx), _currentUnit(nullptr) {

}

UsageAnalysis::~UsageAnalysis() {

}

void UsageAnalysis::visit(Program* prog) {
    ASTImplicitVisitor::visit(prog);

    for (const std::pair<sym::VariableSymbol*, std::vector<Identifier*>>& var : _undeclaredVars) {
        for (Identifier* ident : var.second) {
            _ctx->reporter().error(*ident, "Variable `" + var.first->getName() + "` is used or assigned before being declared");
        }
    }

    for (const std::pair<ClassDecl*, std::vector<InstantiationInfo>>& inst : _classInstantiationsToUnits) {
        if (CapturesUserData* capturesData = inst.first->getUserdata<CapturesUserData>()) {
            if (capturesData->captures.size() > 0) {
                ASTNode* unitDeclaredIn = capturesData->unitDeclaredIn;

                for (InstantiationInfo info : inst.second) {
                    if (info.instUnit != unitDeclaredIn) {
                        // because the actual name of the instantiated class can be different
                        // from inst.first in case of inheritance
                        std::string className = type::getIf<type::ProperType>(
                                    ASTTypeCreator::createType(info.instNode->getInstantiatedExpression(), _ctx)
                                    ->applyTCCallsOnly(_ctx))->getClass()->getName();

                        _ctx->reporter().error(*info.instNode, std::string("Class `") + className +
                                               "` cannot be instantiated here because it captures variables " +
                                               "from its declaration scope which are not available in this context");
                    }
                }
            }
        }
    }
}

void UsageAnalysis::visit(ClassDecl* clss) {
    SAVE_UNDECLARED_VARS

    for (TypeSpecifier* tps : clss->getFields()) {
        if (sym::VariableSymbol* var = sym::getIfSymbolOfType<sym::VariableSymbol>(tps->getSpecified()->getSymbol())) {
            var->setProperty(UsageProperty::DECLARED | UsageProperty::INITIALIZED | UsageProperty::USED);
        }
    }

    ASTImplicitVisitor::visit(clss);

    SET_CAPTURES_AND_UPDATE_UNDECLARED_VARS(clss)
}

void UsageAnalysis::visit(DefineDecl* def) {
    SAVE_MEMBER_AND_SET(_currentUnit, def)

    ASTImplicitVisitor::visit(def);

    RESTORE_MEMBER(_currentUnit)

    LocalUsageAnalysis analyser(_ctx, _undeclaredVars);
    analyser.analyse(def);
}

void UsageAnalysis::visit(FunctionCreation* func) {
    SAVE_UNDECLARED_VARS

    SAVE_MEMBER_AND_SET(_currentUnit, func)

    ASTImplicitVisitor::visit(func);

    RESTORE_MEMBER(_currentUnit)

    LocalUsageAnalysis analyser(_ctx, _undeclaredVars);
    analyser.analyse(func);

    SET_CAPTURES_AND_UPDATE_UNDECLARED_VARS(func)
}

void UsageAnalysis::visit(Instantiation* inst) {
    ASTImplicitVisitor::visit(inst);

    TypeExpression* instantiatedExpr = inst->getInstantiatedExpression();

    while (true) {
        if (type::Type* tp = ASTTypeCreator::createType(instantiatedExpr, _ctx)) {
            if (type::ProperType* pt = type::getIf<type::ProperType>(tp->applyTCCallsOnly(_ctx))) {
                _classInstantiationsToUnits[pt->getClass()].push_back(InstantiationInfo(inst, _currentUnit));
                if ((instantiatedExpr = pt->getClass()->getParent())) {
                    continue;
                }
            }
        }
        break;
    }
}

UsageAnalysis::InstantiationInfo::InstantiationInfo(Instantiation* instNode, ASTNode* instUnit)
    : instNode(instNode), instUnit(instUnit) {

}

}

}
