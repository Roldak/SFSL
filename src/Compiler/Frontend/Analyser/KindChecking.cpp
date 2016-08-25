//
//  TypeChecking.cpp
//  SFSL
//
//  Created by Romain Beguet on 30.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "KindChecking.h"
#include "../AST/Visitors/ASTTypeIdentifier.h"
#include "../AST/Visitors/ASTTypeCreator.h"
#include "../AST/Visitors/ASTKindCreator.h"
#include "../AST/Visitors/ASTSymbolExtractor.h"

#include "../Symbols/Scope.h"

namespace sfsl {

namespace ast {

// TYPE CHECK

KindChecking::KindChecking(CompCtx_Ptr& ctx)
    : ASTImplicitVisitor(ctx), _rep(ctx->reporter()), _mustDefer(true), _insideMemberAccess(false) {

}

KindChecking::~KindChecking() {

}

void KindChecking::visit(ASTNode*) {

}

void KindChecking::visit(Program* prog) {
    ASTImplicitVisitor::visit(prog);
    visitDeferredExpressions();
}

void KindChecking::visit(TypeDecl* tdecl) {
    if (TRY_INSERT(_visitedTypeDefs, tdecl)) {
        tdecl->getExpression()->onVisit(this);

        // kind inference
        tdecl->getName()->setKind(tdecl->getExpression()->kind());
        tdecl->getSymbol()->setKind(tdecl->getExpression()->kind());
    }
}

void KindChecking::visit(ClassDecl* clss) {
    clss->setKind(createProperKindWithBounds(clss, clss));

    if (_mustDefer) {
        _deferredExpressions.emplace(clss);
    } else {
        _mustDefer = true;
        ASTImplicitVisitor::visit(clss);
        _mustDefer = false;

        if (TypeExpression* p = clss->getParent()) {
            if (p->kind()->getKindGenre() != kind::KIND_PROPER) {
                _rep.error(*p, "Kind mismatch. Expected proper type, found type of kind " + p->kind()->toString());
            }
        }
    }
}

void KindChecking::visit(DefineDecl* decl) {
    ASTImplicitVisitor::visit(decl);

    if (TypeExpression* expr = decl->getTypeSpecifier()) {
        if (expr->kind()->getKindGenre() != kind::KIND_PROPER) {
            _rep.error(*expr, "Kind mismatch. Expected proper type, found type of kind " + expr->kind()->toString());
        }
    }
}

void KindChecking::visit(FunctionTypeDecl* ftdecl) {
    ASTImplicitVisitor::visit(ftdecl);

    for (TypeExpression* texpr : ftdecl->getArgTypes()) {
        if (texpr->kind()->getKindGenre() != kind::KIND_PROPER) {
            _rep.error(*texpr, "Kind mismatch. Expected proper type, found type of kind " + texpr->kind()->toString());
        }
    }

    if (ftdecl->getRetType()->kind()->getKindGenre() != kind::KIND_PROPER) {
        _rep.error(*ftdecl->getRetType(), "Kind mismatch. Expected proper type, found type of kind " + ftdecl->getRetType()->kind()->toString());
    }

    ftdecl->setKind(createProperKindWithBounds(ftdecl, ftdecl));
}

void KindChecking::visit(TypeMemberAccess* tdot) {
    SAVE_MEMBER_AND_SET(_insideMemberAccess, true)

    tdot->getAccessed()->onVisit(this);

    RESTORE_MEMBER(_insideMemberAccess)

    if (tdot->getSymbol()) {
        trySetKindOfSymbolic(tdot);
    }
}

void KindChecking::visit(TypeTuple* ttuple) {
    ASTImplicitVisitor::visit(ttuple);
}

void KindChecking::visit(TypeConstructorCreation* tc) {
    ASTImplicitVisitor::visit(tc);

    std::vector<TypeExpression*> exprs;

    if (TypeTuple* ttuple = getIfNodeOfType<TypeTuple>(tc->getArgs(), _ctx)) { // form is `[] => ...` or `[exp, exp] => ...`, ...
        exprs = ttuple->getExpressions();
    } else { // form is `exp => ...` or `[exp] => ...`
        exprs.push_back(tc->getArgs());
    }

    std::vector<kind::TypeConstructorKind::Parameter> params(exprs.size());
    kind::Kind* retKind;

    for (size_t i = 0; i < exprs.size(); ++i) {
        params[i].varianceType = common::VAR_T_NONE;
        if (TypeParameter* tparam = getIfNodeOfType<TypeParameter>(exprs[i], _ctx)) {
            params[i].varianceType = tparam->getVarianceType();
        }
        params[i].kind = exprs[i]->kind();
    }

    retKind = tc->getBody()->kind();

    tc->setKind(_mngr.New<kind::TypeConstructorKind>(params, retKind));
}

void KindChecking::visit(TypeConstructorCall* tcall) {
    ASTImplicitVisitor::visit(tcall);

    if (kind::TypeConstructorKind* k = kind::getIf<kind::TypeConstructorKind>(tcall->getCallee()->kind())) {
        if (type::TypeConstructorType* ctrType = type::getIf<type::TypeConstructorType>(
                    ASTTypeCreator::createType(tcall->getCallee(), _ctx)->applyTCCallsOnly(_ctx))) {

            // retrive the call arguments and their types
            const std::vector<TypeExpression*>& callArgs = tcall->getArgs();
            std::vector<type::Type*> callArgsTypes(callArgs.size());

            for (size_t i = 0; i < callArgs.size(); ++i) {
                callArgsTypes[i] = ASTTypeCreator::createType(callArgs[i], _ctx);
            }

            // retrieve parameters of the type constructor and their kinds
            TypeExpression* paramsExpr = ctrType->getTypeConstructor()->getArgs();
            std::vector<TypeExpression*> params;

            if (TypeTuple* ttuple = getIfNodeOfType<TypeTuple>(paramsExpr, _ctx)) { // form is `[] => ...` or `[exp, exp] => ...`, ...
                params = ttuple->getExpressions();
            } else { // form is `exp => ...` or `[exp] => ...`
                params.push_back(paramsExpr);
            }

            std::vector<kind::Kind*> paramKinds(params.size());
            for (size_t i = 0; i < params.size(); ++i) {
                paramKinds[i] = params[i]->kind();
            }

            // build the environment of the type constructor being called

            type::Environment ctrEnv(ctrType->getEnvironment());
            type::Environment callEnv(ASTTypeCreator::buildEnvironmentFromTypeParameterInstantiation(params, callArgsTypes, _ctx));
            ctrEnv.insert(callEnv.begin(), callEnv.end());

            // kind check

            if (kindCheckWithBoundsArgumentSubstitution(paramKinds, callArgs, callArgsTypes, *tcall, ctrEnv, _ctx)) {
                tcall->setKind(k->getRetKind());
            } else {
                tcall->setKind(kind::Kind::NotYetDefined());
            }
        } else {
            _ctx->reporter().fatal(*tcall->getCallee(), "Callee was not a type constructor");
        }
    }
    else {
        _rep.error(*tcall->getCallee(), "Type expression " + tcall->getCallee()->kind()->toString() + " cannot be called.");
    }
}

void KindChecking::visit(TypeIdentifier* tident) {
    trySetKindOfSymbolic(tident);
}

void KindChecking::visit(TypeToBeInferred* tbi) {
    tbi->setKind(kind::ProperKind::create());
}

void KindChecking::visit(TypeParameter* tparam) {
    ASTImplicitVisitor::visit(tparam);
    kind::Kind* k;
    if (!(k = ASTKindCreator::createKind(tparam->getKindNode(), _ctx))) {
        k = kind::Kind::NotYetDefined();
    }
    tparam->getSpecified()->setKind(k);
    tparam->setKind(k);
}

void KindChecking::visit(TypeSpecifier* ts) {
    ASTImplicitVisitor::visit(ts);

    if (ts->getTypeNode()->kind()->getKindGenre() != kind::KIND_PROPER) {
        _rep.error(*ts, "Variables must have a proper type. Found a type of kind " + ts->getTypeNode()->kind()->toString());
    }
}

void KindChecking::visit(Instantiation* inst) {
    ASTImplicitVisitor::visit(inst);

    if (inst->getInstantiatedExpression()->kind()->getKindGenre() != kind::KIND_PROPER) {
        _rep.error(*inst, "Only proper types can be instantiated. Found type of kind " + inst->getInstantiatedExpression()->kind()->toString());
    }
}

bool KindChecking::kindCheckArgumentSubstitution(const std::vector<kind::Kind*>& parametersKinds,
                                                 const std::vector<TypeExpression*>& arguments,
                                                 const common::Positionnable& callPos, CompCtx_Ptr& ctx) {
    if (arguments.size() != parametersKinds.size()) {
        ctx->reporter().error(callPos, "Wrong number of type arguments. Expected " +
                   utils::T_toString(parametersKinds.size()) + ", found " + utils::T_toString(arguments.size()));
        return false;
    }

    for (size_t i = 0; i < arguments.size(); ++i) {
        if (!arguments[i]->kind()->isSubKindOf(parametersKinds[i], ctx)) {
            ctx->reporter().error(*arguments[i], "Kind mismatch. Expected " + parametersKinds[i]->toString() +
                       ", found " + arguments[i]->kind()->toString());
            return false;
        }
    }

    return true;
}

bool KindChecking::kindCheckWithBoundsArgumentSubstitution(const std::vector<kind::Kind*>& parametersKinds,
                                                           const std::vector<TypeExpression*>& arguments,
                                                           const std::vector<type::Type*>& createdArguments,
                                                           const common::Positionnable& callPos,
                                                           const type::Environment& env, CompCtx_Ptr& ctx,
                                                           bool reportErrors) {
    if (arguments.size() != parametersKinds.size()) {
        if (reportErrors) {
            ctx->reporter().error(callPos, "Wrong number of type arguments. Expected " +
                       utils::T_toString(parametersKinds.size()) + ", found " + utils::T_toString(arguments.size()));
        }
        return false;
    }

    for (size_t i = 0; i < arguments.size(); ++i) {
        kind::Kind* appliedArgKind = arguments[i]->kind()->substitute(createdArguments[i]->apply(ctx)->getEnvironment(), ctx)->apply(ctx);
        kind::Kind* appliedParamKind = parametersKinds[i]->substitute(env, ctx)->apply(ctx);
        if (!appliedArgKind->isSubKindOf(appliedParamKind, ctx, true)) {
            if (reportErrors) {
                ctx->reporter().error(*arguments[i], "Argument kind mismatch. Expected " + appliedParamKind->toString(true, &ctx) +
                           ", found " + appliedArgKind->toString(true, &ctx));
            }
            return false;
        }
    }

    return true;
}

kind::ProperKind* KindChecking::createProperKindWithBounds(TypeExpression* lb, TypeExpression* ub) {
    type::ProperType* lbType = nullptr;
    type::ProperType* ubType = nullptr;

    if (lb) {
        if (type::Type* tp = ASTTypeCreator::createType(lb, _ctx)) {
            lbType = type::getIf<type::ProperType>(tp);
        }
    }

    if (ub) {
        if (ub == lb) {
            ubType = lbType;
        } else if (type::Type* tp = ASTTypeCreator::createType(ub, _ctx)) {
            ubType = type::getIf<type::ProperType>(tp);
        }
    }

    return (!lbType && !ubType) ? kind::ProperKind::create()
                                : _mngr.New<kind::ProperKind>(lbType, ubType);
}

void KindChecking::visitDeferredExpressions() {
    while (!_deferredExpressions.empty()) {
        std::set<TypeExpression*> copy = _deferredExpressions;
        _deferredExpressions.clear();

        for (TypeExpression* expr : copy) {
            _mustDefer = false;
            expr->onVisit(this);
        }
    }
}

template<typename T>
void KindChecking::trySetKindOfSymbolic(T* symbolic) {
    if (symbolic->getSymbolCount() != 1) {
        _rep.error(*symbolic, "Symbolic refers to zero or several type symbols at the same time");
        return;
    }

    if (sym::Symbol* s = symbolic->getSymbol()) {
        if (kind::Kind* k = tryGetKindOfSymbol(s)) {
            symbolic->setKind(k);
        } else if (!_insideMemberAccess) {
            _rep.error(*symbolic, "Symbol " + s->getName() + " does not have any kind");
        }
    }
}

kind::Kind* KindChecking::tryGetKindOfSymbol(sym::Symbol* sym) {
    if (sym::TypeSymbol* tpesym = sym::getIfSymbolOfType<sym::TypeSymbol>(sym)) {
        tpesym->getTypeDecl()->onVisit(this);

        if (tpesym->kind() == kind::Kind::NotYetDefined()) {
            _rep.error(*sym, "Kind of " + sym->getName() + " cannot be inferred because of a cyclic dependency");
        }

        return tpesym->kind();
    }

    return nullptr;
}

}

}
