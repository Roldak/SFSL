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

namespace sfsl {

namespace ast {

ASTTypeCreator::ASTTypeCreator(CompCtx_Ptr& ctx, const std::vector<type::Type*>& args)
    : ASTExplicitVisitor(ctx), _created(nullptr), _args(args) {

}

ASTTypeCreator::~ASTTypeCreator() {

}

void ASTTypeCreator::visit(ASTNode*) {
    // do not throw an exception
}

void ASTTypeCreator::visit(ClassDecl* clss) {
    _created = _mngr.New<type::ProperType>(clss, buildSubstitutionTableFromTypeParametrizable(clss));
}

void ASTTypeCreator::visit(FunctionTypeDecl* ftdecl) {
    const std::vector<TypeExpression*>& argTypeExprs(ftdecl->getArgTypes());

    std::vector<type::Type*> argTypes(argTypeExprs.size());
    type::Type* retType = createType(ftdecl->getRetType(), _ctx);

    for (size_t i = 0; i < argTypes.size(); ++i) {
        argTypes[i] = createType(argTypeExprs[i], _ctx);
    }

    ClassDecl* functionClass = nullptr;
    type::SubstitutionTable table;

    if (type::ProperType* pt = type::getIf<type::ProperType>(createType(ftdecl->getClassEquivalent(), _ctx)->applyTCCallsOnly(_ctx))) {
        functionClass = pt->getClass();
        table = pt->getSubstitutionTable();
    } else {
        _ctx->reporter().fatal(*ftdecl, "Could not create type of this function's class equivalent");
    }

    _created = _mngr.New<type::FunctionType>(std::vector<TypeExpression*>(), argTypes, retType, functionClass, table);
}

void ASTTypeCreator::visit(TypeConstructorCreation* typeconstructor) {
    _created = _mngr.New<type::TypeConstructorType>(typeconstructor, buildSubstitutionTableFromTypeParametrizable(typeconstructor));
}

void ASTTypeCreator::visit(TypeConstructorCall* tcall) {
    tcall->getCallee()->onVisit(this);
    type::Type* ctr = _created;

    const std::vector<TypeExpression*>& found = tcall->getArgs();
    std::vector<type::Type*> args(found.size());

    for (size_t i = 0; i < found.size(); ++i) {
        if (!(args[i] = createType(found[i], _ctx))) {
            _ctx->reporter().fatal(*found[i], "failed to create type");
            _created = nullptr;
            return;
        }
    }

    _created = _mngr.New<type::ConstructorApplyType>(ctr, args, buildSubstitutionTableFromTypeParametrizable(tcall));
}

void ASTTypeCreator::visit(TypeMemberAccess* mac) {
    mac->getAccessed()->onVisit(this);

    if (_created) {
        if (type::ProperType* pt = type::getIf<type::ProperType>(_created->applyTCCallsOnly(_ctx))) {
            sym::Scope* classScope = pt->getClass()->getScope();
            if (classScope->assignSymbolic(*mac->getMember(), mac->getMember()->getValue())) {
                mac->getMember()->onVisit(this);
                if (_created) {
                    _created = _created->substitute(pt->getSubstitutionTable(), _ctx);
                }
            }
        }
    } else {
        createTypeFromSymbolic(mac, *mac);
    }
}

void ASTTypeCreator::visit(TypeIdentifier* ident) {
    createTypeFromSymbolic(ident, *ident);
}

void ASTTypeCreator::visit(Identifier* ident) {
    createTypeFromSymbolic(ident, *ident);
}

type::Type* ASTTypeCreator::getCreatedType() const {
    return _created;
}

type::Type* ASTTypeCreator::createType(ASTNode* node, CompCtx_Ptr& ctx) {
    ASTTypeCreator creator(ctx, {});
    node->onVisit(&creator);
    return creator.getCreatedType();
}

type::Type* ASTTypeCreator::evalTypeConstructor(type::TypeConstructorType* ctr, const std::vector<type::Type*>& args, CompCtx_Ptr& ctx) {
    TypeExpression* argsExpr = ctr->getTypeConstructor()->getArgs();
    std::vector<TypeExpression*> params;

    if (isNodeOfType<TypeTuple>(argsExpr, ctx)) { // form is `[] => ...` or `[exp, exp] => ...`, ...
        params = static_cast<TypeTuple*>(argsExpr)->getExpressions();
    } else { // form is `exp => ...` or `[exp] => ...`
        params.push_back(argsExpr);
    }

    if (params.size() != args.size()) { // can happen if this is called before kind checking occurs
        return type::Type::NotYetDefined();
    }

    type::Type* created = createType(ctr->getTypeConstructor()->getBody(), ctx);
    type::SubstitutionTable subs(buildSubstitutionTableFromTypeParameterInstantiation(params, args, ctx));

    created = type::Type::findSubstitution(subs, created)->substitute(subs, ctx);
    created = type::Type::findSubstitution(ctr->getSubstitutionTable(), created)->substitute(ctr->getSubstitutionTable(), ctx);

    return created;
}

type::Type* ASTTypeCreator::evalFunctionConstructor(type::Type* fc, const std::vector<type::Type*>& args, CompCtx_Ptr& ctx) {

    std::vector<TypeExpression*> typeArgs;
    type::Type* created;

    if (type::FunctionType* ft = type::getIf<type::FunctionType>(fc)) {
        typeArgs = ft->getTypeArgs();
        created = ctx->memoryManager().New<type::FunctionType>(
                        std::vector<TypeExpression*>(), ft->getArgTypes(), ft->getRetType(), ft->getClass(), ft->getSubstitutionTable());
    } else if (type::MethodType* mt = type::getIf<type::MethodType>(fc)) {
        typeArgs = mt->getTypeArgs();
        created = ctx->memoryManager().New<type::MethodType>(
                        mt->getOwner(), std::vector<TypeExpression*>(), mt->getArgTypes(), mt->getRetType(), mt->getSubstitutionTable());
    } else {
        return type::Type::NotYetDefined();
    }

    if (typeArgs.size() != args.size()) { // can happen if this is called before kind checking occurs
        return type::Type::NotYetDefined();
    } else if (typeArgs.size() == 0) {
        return fc;
    }

    type::SubstitutionTable subs(buildSubstitutionTableFromTypeParameterInstantiation(typeArgs, args, ctx));

    created = type::Type::findSubstitution(subs, created)->substitute(subs, ctx);
    created = type::Type::findSubstitution(fc->getSubstitutionTable(), created)->substitute(fc->getSubstitutionTable(), ctx);

    return created;
}

void ASTTypeCreator::createTypeFromSymbolic(sym::Symbolic<sym::Symbol>* symbolic, common::Positionnable& pos) {
    std::vector<sym::TypeSymbol*> typeSyms;
    for (const auto& symData : symbolic->getSymbolDatas()) {
        if (symData.symbol->getSymbolType() == sym::SYM_TPE) {
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
            ts->getTypeDecl()->getExpression()->onVisit(this);
            ts->setType(_created);
        } else {
            _ctx->reporter().error(pos, "A cyclic dependency was found");
        }

    } else {
        _created = ts->type();
    }
}

type::SubstitutionTable ASTTypeCreator::buildSubstitutionTableFromTypeParametrizable(type::TypeParametrizable* param) {
    type::SubstitutionTable table;
    const std::vector<sym::TypeSymbol*>& syms(param->getDependencies());

    for (sym::TypeSymbol* ts : syms) {
        table.insert(std::make_pair(ts->type(), ts->type()));
    }

    return table;
}

type::SubstitutionTable ASTTypeCreator::buildSubstitutionTableFromTypeParameterInstantiation(
        std::vector<TypeExpression*> params, const std::vector<type::Type*>& args, CompCtx_Ptr& ctx) {

    for (TypeExpression*& param : params) {
        if (isNodeOfType<KindSpecifier>(param, ctx)) {
            param = static_cast<KindSpecifier*>(param)->getSpecified();
        }
    }

    type::SubstitutionTable subs;

    for (size_t i = 0; i < params.size(); ++i) {
        // can only be a TypeSymbol
        sym::TypeSymbol* param = static_cast<sym::TypeSymbol*>(ASTSymbolExtractor::extractSymbol(params[i], ctx));

        subs[param->type()] = args[i]->apply(ctx);
    }

    return subs;
}

}

}
