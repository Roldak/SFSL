//
//  ASTTypeCreator.cpp
//  SFSL
//
//  Created by Romain Beguet on 31.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "ASTTypeCreator.h"
#include "ASTTypeIdentifier.h"
#include "ASTSymbolExtractor.h"

#include "../../Symbols/Scope.h"

#include "../Utils/HasCacheableCreatedType.h"

#include "../../Analyser/KindChecking.h"

namespace sfsl {

namespace ast {

ASTTypeCreator::ASTTypeCreator(CompCtx_Ptr& ctx)
    : ASTExplicitVisitor(ctx), _created(nullptr) {

}

ASTTypeCreator::~ASTTypeCreator() {

}

void ASTTypeCreator::visit(ASTNode*) {
    // do not throw an exception
}

void ASTTypeCreator::visit(ClassDecl* clss) {
    _created = _mngr.New<type::ProperType>(clss, buildEnvironmentFromTypeParametrizable(clss));
}

void ASTTypeCreator::visit(FunctionTypeDecl* ftdecl) {
    const std::vector<TypeExpression*>& argTypeExprs(ftdecl->getArgTypes());

    std::vector<type::Type*> argTypes(argTypeExprs.size());
    type::Type* retType = createType(ftdecl->getRetType());

    for (size_t i = 0; i < argTypes.size(); ++i) {
        argTypes[i] = createType(argTypeExprs[i]);
    }

    ClassDecl* functionClass = nullptr;
    type::Environment env;

    if (ftdecl->getTypeArgs().size() == 0) {
        if (type::ProperType* pt = type::getIf<type::ProperType>(createType(ftdecl->getClassEquivalent())->applyTCCallsOnly(_ctx))) {
            functionClass = pt->getClass();
            env = pt->getEnvironment();
        } else {
            _ctx->reporter().fatal(*ftdecl, "Could not create type of this function's class equivalent");
        }
    }

    _created = _mngr.New<type::FunctionType>(ftdecl->getTypeArgs(), argTypes, retType, functionClass, env);
}

void ASTTypeCreator::visit(TypeConstructorCreation* typeconstructor) {
    _created = _mngr.New<type::TypeConstructorType>(typeconstructor, buildEnvironmentFromTypeParametrizable(typeconstructor));
}

void ASTTypeCreator::visit(TypeConstructorCall* tcall) {
    doVisit(tcall->getCallee());
    type::Type* ctr = _created;

    const std::vector<TypeExpression*>& found = tcall->getArgs();
    std::vector<type::Type*> args(found.size());

    for (size_t i = 0; i < found.size(); ++i) {
        if (!(args[i] = createType(found[i]))) {
            _ctx->reporter().fatal(*found[i], "failed to create type");
            _created = nullptr;
            return;
        }
    }

    _created = _mngr.New<type::ConstructorApplyType>(ctr, args);
}

void ASTTypeCreator::visit(TypeMemberAccess* mac) {
    createTypeFromMemberAccess(mac);
}

void ASTTypeCreator::visit(TypeIdentifier* ident) {
    createTypeFromSymbolic(ident, *ident);
}

void ASTTypeCreator::visit(MemberAccess* mac) {
    createTypeFromMemberAccess(mac);
}

void ASTTypeCreator::visit(Identifier* ident) {
    createTypeFromSymbolic(ident, *ident);
}

type::Type* ASTTypeCreator::createType(ASTNode* node, CompCtx_Ptr& ctx) {
    return ASTTypeCreator(ctx).createType(node);
}

type::Type* ASTTypeCreator::createType(ASTNode* node) {
    SAVE_MEMBER_AND_SET(_created, nullptr)

    doVisit(node);
    type::Type* created = _created;

    RESTORE_MEMBER(_created)

    return created;
}

void ASTTypeCreator::doVisit(ASTNode* node) {
    HasCacheableCreatedType* cacheable = getIfNodeOfType<TypeExpression>(node, _ctx);

    if (cacheable && cacheable->hasCachedType()) {
        _created = cacheable->getCachedType();
        return;
    }

    node->onVisit(this);

    if (cacheable) {
        cacheable->setCachedType(_created);
    }
}

type::Type* ASTTypeCreator::evalTypeConstructor(type::TypeConstructorType* ctr, const std::vector<type::Type*>& args, CompCtx_Ptr& ctx) {
    TypeExpression* argsExpr = ctr->getTypeConstructor()->getArgs();
    std::vector<TypeExpression*> params;

    if (TypeTuple* ttuple = getIfNodeOfType<TypeTuple>(argsExpr, ctx)) { // form is `[] => ...` or `[exp, exp] => ...`, ...
        params = ttuple->getExpressions();
    } else { // form is `exp => ...` or `[exp] => ...`
        params.push_back(argsExpr);
    }

    if (params.size() != args.size()) { // can happen if this is called before kind checking occurs
        return type::Type::NotYetDefined();
    }

    type::Type* created = createType(ctr->getTypeConstructor()->getBody(), ctx);
    type::Environment subs(buildEnvironmentFromTypeParameterInstantiation(params, args, ctx));

    created = created->substitute(subs, ctx)->substitute(ctr->getEnvironment(), ctx);

    return created;
}

TypeExpression* typeExpressionFromType(type::Type* tp) {
    if (type::ProperType* pt = type::getIf<type::ProperType>(tp)) {
        return pt->getClass();
    } else if (type::TypeConstructorType* tct = type::getIf<type::TypeConstructorType>(tp)) {
        return tct->getTypeConstructor();
    }
    return nullptr;
}

bool unify(const type::Type* ta, type::Type* tb, const std::vector<type::Type*>& params,
           std::vector<TypeExpression*>& args, std::vector<type::Type*>& argTypes, CompCtx_Ptr& ctx) {

    for (size_t i = 0; i < params.size(); ++i) {
        if (ta == params[i]) {
            // unification fails if some parameter was already assigned to a different type
            if (argTypes[i] != type::Type::NotYetDefined() && !argTypes[i]->equals(tb, ctx)) {
                return false;
            }
            args[i] = typeExpressionFromType(tb->applyTCCallsOnly(ctx));
            argTypes[i] = tb;
            return true;
        }
    }

    if (type::ConstructorApplyType* appA = type::getIf<type::ConstructorApplyType>(ta)) {
        if (type::ConstructorApplyType* appB = type::getIf<type::ConstructorApplyType>(tb)) {
            if (appA->getArgs().size() == appB->getArgs().size()) {
                if (unify(appA->getCallee(), appB->getCallee(), params, args, argTypes, ctx)) {
                    bool ok = true;
                    for (size_t i = 0; i < appA->getArgs().size(); ++i) {
                        if (!unify(appA->getArgs()[i], appB->getArgs()[i], params, args, argTypes, ctx)) {
                            ok = false;
                            break;
                        }
                    }
                    if (ok) {
                        return true;
                    }
                }
            }
        }
        if (type::ProperType* pB = type::getIf<type::ProperType>(tb->applyTCCallsOnly(ctx))) {
            if (pB->getClass()->getParent()) {
                if (type::Type* parent = ASTTypeCreator::createType(pB->getClass()->getParent(), ctx)) {
                    return unify(ta, parent->substitute(pB->getEnvironment(), ctx), params, args, argTypes, ctx);
                }
            }
        }
        return false;
    }

    if (type::ValueConstructorType* valA = type::getIf<type::ValueConstructorType>(ta)) {
        if (type::ValueConstructorType* valB = type::getIf<type::ValueConstructorType>(tb)) {
            if (valA->getArgTypes().size() == valB->getArgTypes().size()) {
                if (unify(valA->getRetType(), valB->getRetType(), params, args, argTypes, ctx)) {
                    for (size_t i = 0; i < valA->getArgTypes().size(); ++i) {
                        if (!unify(valA->getArgTypes()[i], valB->getArgTypes()[i], params, args, argTypes, ctx)) {
                            return false;
                        }
                    }
                    return true;
                }
            }
        }
        return false;
    }

    return ta->equals(tb, ctx);
}

bool checkArgumentsValidity(const std::vector<type::Type*>& args) {
    for (type::Type* arg : args) {
        if (arg->getTypeKind() == type::TYPE_NYD) {
            return false;
        }
    }
    return true;
}

type::Type* ASTTypeCreator::evalFunctionConstructor(type::Type* fc, const std::vector<TypeExpression*>& args,
                                                    const common::Positionnable& callPos, CompCtx_Ptr& ctx,
                                                    ArgTypeEvaluator* callArgTypes, bool reportErrors) {

    std::vector<TypeExpression*> typeParams;
    const std::vector<type::Type*>* paramTypes;
    type::Type* created;

    if (type::ValueConstructorType* vt = type::getIf<type::ValueConstructorType>(fc)) {
        typeParams = vt->getTypeArgs();
        paramTypes = &vt->getArgTypes();

        if (typeParams.size() == 0 && args.size() == 0) {
            return fc;
        }

        created = dynamic_cast<type::Type*>(vt->rebuildValueConstructor({}, vt->getArgTypes(), vt->getRetType(), fc->getEnvironment(), ctx));
    } else {
        return type::Type::NotYetDefined();
    }

    std::vector<kind::Kind*> paramKinds;
    for (const TypeExpression* expr : typeParams) {
        paramKinds.push_back(expr->kind());
    }

    std::vector<type::Type*> argTypes(args.size());
    for (size_t i = 0; i < args.size(); ++i) {
        if (!(argTypes[i] = ASTTypeCreator::createType(args[i], ctx))) {
            argTypes[i] = type::Type::NotYetDefined();
        }
    }

    std::vector<TypeExpression*> argExprs(args);

    if (typeParams.size() > 0 && args.size() == 0 && callArgTypes) {
        // Caller supplied 0 type arguments, so try to infer them
        argExprs.resize(typeParams.size(), nullptr);
        argTypes.resize(typeParams.size(), type::Type::NotYetDefined());

        std::vector<type::Type*> typeParamTypes(typeParams.size());
        for (size_t i = 0; i < typeParams.size(); ++i) {
            TypeExpression* param = typeParams[i];
            if (TypeParameter* tparam = getIfNodeOfType<TypeParameter>(param, ctx)) {
                param = tparam->getSpecified();
            }

            typeParamTypes[i] = static_cast<sym::TypeSymbol*>(ASTSymbolExtractor::extractSymbol(param, ctx))->type();
        }

        if (paramTypes->size() != callArgTypes->size()) {
            if (reportErrors) {
                ctx->reporter().error(callPos,
                           "Wrong number of argument. Found " + utils::T_toString(callArgTypes->size()) +
                           ", expected " + utils::T_toString(paramTypes->size()));
            }
            return type::Type::NotYetDefined();
        }

        for (size_t i = 0; i < paramTypes->size(); ++i) {
            type::Environment envSoFar(buildEnvironmentFromTypeParameterInstantiation(typeParams, argTypes, ctx));
            type::Type* expectedType = paramTypes->at(i)->substitute(envSoFar, ctx);

            if (  !unify(paramTypes->at(i), callArgTypes->at(i, expectedType), typeParamTypes, argExprs, argTypes, ctx) ||
                   checkArgumentsValidity(argTypes)) {
                break;
            }
        }

        if (!checkArgumentsValidity(argTypes)) {
            if (reportErrors) {
                ctx->reporter().error(callPos, "Unable to infer type arguments based on call arguments");
            }
            return type::Type::NotYetDefined();
        }
    }

    type::Environment fnEnv(created->getEnvironment());
    type::Environment callEnv(buildEnvironmentFromTypeParameterInstantiation(typeParams, argTypes, ctx));
    fnEnv.insert(callEnv.begin(), callEnv.end());

    if (!KindChecking::kindCheckWithBoundsArgumentSubstitution(paramKinds, argExprs, argTypes, callPos, fnEnv, ctx, reportErrors)) {
        return type::Type::NotYetDefined();
    } else if (typeParams.size() == 0) {
        return fc;
    }

    created = created->substitute(callEnv, ctx);

    return created;
}

template<typename T>
void ASTTypeCreator::createTypeFromMemberAccess(T* mac) {
    doVisit(mac->getAccessed());

    if (_created) {
        if (type::ProperType* pt = type::getIf<type::ProperType>(_created->applyTCCallsOnly(_ctx))) {
            sym::Scope* classScope = pt->getClass()->getScope();
            if (classScope->assignSymbolic(*mac->getMember(), mac->getMember()->getValue())) {
                doVisit(mac->getMember());
                if (_created) {
                    _created = _created->substitute(pt->getEnvironment(), _ctx);
                }
            }
        }
    } else {
        createTypeFromSymbolic(mac, *mac);
    }
}

void ASTTypeCreator::createTypeFromSymbolic(sym::Symbolic<sym::Symbol>* symbolic, common::Positionnable& pos) {
    std::vector<sym::TypeSymbol*> typeSyms;
    for (const auto& symData : symbolic->getSymbolDatas()) {
        if (symData.symbol && symData.symbol->getSymbolType() == sym::SYM_TPE) {
            typeSyms.push_back(static_cast<sym::TypeSymbol*>(symData.symbol));
        }
    }

    if (typeSyms.empty()) {
        return;
    } else if (typeSyms.size() > 1) {
        _ctx->reporter().error(pos, "Symbolic refers to multiple type symbols (" + utils::T_toString(typeSyms.size()) + " found)");
        for (const sym::TypeSymbol* ts : typeSyms) {
            _ctx->reporter().info(*ts, "This one");
        }
    }

    sym::TypeSymbol* ts = typeSyms.front();

    if (ts->type() == type::Type::NotYetDefined()) {

        if (TRY_INSERT(_visitedTypes, ts)) {
            doVisit(ts->getTypeDecl()->getExpression());
            ts->setType(_created);
        } else {
            _ctx->reporter().error(pos, "A cyclic dependency was found");
        }

    } else {
        _created = ts->type();
    }
}

type::Environment ASTTypeCreator::buildEnvironmentFromTypeParametrizable(type::TypeParametrizable* param) {
    type::Environment env;
    const std::vector<type::TypeParametrizable::Parameter>& syms(param->getParameters());

    for (type::TypeParametrizable::Parameter ts : syms) {
        env.insert(type::Environment::Substitution(ts.varianceType, ts.symbol->type(), ts.symbol->type()));
    }

    return env;
}

type::Environment ASTTypeCreator::buildEnvironmentFromTypeParameterInstantiation(
        std::vector<TypeExpression*> params, const std::vector<type::Type*>& args, CompCtx_Ptr& ctx) {

    if (params.size() != args.size()) {
        return type::Environment::Empty;
    }

    for (TypeExpression*& param : params) {
        if (TypeParameter* tparam = getIfNodeOfType<TypeParameter>(param, ctx)) {
            param = tparam->getSpecified();
        }
    }

    type::Environment subs;

    for (size_t i = 0; i < params.size(); ++i) {
        // can only be a TypeSymbol
        sym::TypeSymbol* param = static_cast<sym::TypeSymbol*>(ASTSymbolExtractor::extractSymbol(params[i], ctx));

        subs[param->type()] = args[i];
    }

    return subs;
}

}

}
