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

namespace sfsl {

namespace ast {

ASTTypeCreator::ASTTypeCreator(CompCtx_Ptr& ctx, const std::vector<type::Type*>& args)
    : ASTImplicitVisitor(ctx), _created(nullptr), _args(args) {

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

    _created = _mngr.New<type::FunctionType>(argTypes, retType, nullptr);
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

    _created = _mngr.New<type::ConstructorApplyType>(ctr, args, *tcall, buildSubstitutionTableFromTypeParametrizable(tcall));
}

void ASTTypeCreator::visit(TypeMemberAccess* mac) {
    createTypeFromSymbolic(mac, *mac);
}

void ASTTypeCreator::visit(TypeIdentifier* ident) {
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

type::Type* ASTTypeCreator::evalTypeConstructor(TypeConstructorCreation* ctr, CompCtx_Ptr& ctx, const std::vector<type::Type*>& args) {
    type::Type* created = createType(ctr->getBody(), ctx);

    TypeExpression* expr = ctr->getArgs();
    std::vector<TypeExpression*> params;

    if (isNodeOfType<TypeTuple>(expr, ctx)) { // form is `[] => ...` or `[exp, exp] => ...`, ...
        params = static_cast<TypeTuple*>(expr)->getExpressions();
    } else { // form is `exp => ...` or `[exp] => ...`
        params.push_back(expr);
    }

    if (params.size() != args.size()) { // can happen if this is called before kind checking occurs
        return type::Type::NotYetDefined();
    }

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

    return type::Type::findSubstitution(subs, created)->substitute(subs, ctx);
}

void ASTTypeCreator::createTypeFromSymbolic(sym::Symbolic<sym::Symbol>* symbolic, common::Positionnable& pos) {
    if (sym::Symbol* s = symbolic->getSymbol()) {
        if (s->getSymbolType() != sym::SYM_TPE) {
            _ctx->reporter().error(pos, "Expression is not a type");
        }

        sym::TypeSymbol* ts = static_cast<sym::TypeSymbol*>(s);

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
}

type::SubstitutionTable ASTTypeCreator::buildSubstitutionTableFromTypeParametrizable(type::TypeParametrizable* param) {
    type::SubstitutionTable table;
    const std::vector<sym::TypeSymbol*>& syms(param->getDependencies());

    for (sym::TypeSymbol* ts : syms) {
        table.insert(std::make_pair(ts->type(), ts->type()));
    }

    return table;
}

}

}
