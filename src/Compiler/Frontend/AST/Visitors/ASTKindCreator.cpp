//
//  ASTKindCreator.cpp
//  SFSL
//
//  Created by Romain Beguet on 14.05.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "ASTKindCreator.h"
#include "../../Symbols/Scope.h"
#include "ASTTypeCreator.h"

namespace sfsl {

namespace ast {

// AST KIND CREATOR

ASTKindCreator::ASTKindCreator(CompCtx_Ptr& ctx)
    : ASTImplicitVisitor(ctx), _created(nullptr) {

}

ASTKindCreator::~ASTKindCreator() {

}

void ASTKindCreator::visit(ASTNode* node) {
    // do not throw an exception
}

void ASTKindCreator::visit(ProperTypeKindSpecifier* ptks) {
    _created = kind::ProperKind::create();
}

void ASTKindCreator::visit(TypeConstructorKindSpecifier* tcks) {
    std::vector<kind::TypeConstructorKind::Parameter> params(tcks->getArgs().size());
    kind::Kind* ret;

    for (size_t i = 0; i < params.size(); ++i) {
        const TypeConstructorKindSpecifier::Parameter& tcParam(tcks->getArgs()[i]);
        tcParam.kindExpr->onVisit(this);

        params[i].varianceType = tcParam.varianceType;
        params[i].kind = _created;
    }

    tcks->getRet()->onVisit(this);
    ret = _created;

    _created = _mngr.New<kind::TypeConstructorKind>(params, ret);
}

kind::Kind* ASTKindCreator::getCreatedKind() const {
    return _created;
}

kind::Kind* ASTKindCreator::createKind(ASTNode* node, CompCtx_Ptr& ctx) {
    ASTKindCreator creator(ctx);
    node->onVisit(&creator);
    return creator.getCreatedKind();
}

// AST DEFAULT TYPE FROM KIND CREATOR

ASTDefaultTypeFromKindCreator::ASTDefaultTypeFromKindCreator(CompCtx_Ptr& ctx, const std::string& name)
    : ASTImplicitVisitor(ctx), _name(name) {

}

ASTDefaultTypeFromKindCreator::ASTDefaultTypeFromKindCreator(CompCtx_Ptr& ctx, const std::string& name,
                                                             const std::vector<sym::TypeSymbol*>& dependencies)
    : ASTImplicitVisitor(ctx), _name(name), _parameters(dependencies) {

}

ASTDefaultTypeFromKindCreator::~ASTDefaultTypeFromKindCreator() {

}

void ASTDefaultTypeFromKindCreator::visit(ASTNode*) {
    // do not throw an exception
}

void ASTDefaultTypeFromKindCreator::visit(ProperTypeKindSpecifier*) {
    ClassDecl* clss = _mngr.New<ClassDecl>(_name, nullptr,
                                           std::vector<TypeDecl*>(),
                                           std::vector<TypeSpecifier*>(),
                                           std::vector<DefineDecl*>(), false);

    clss->setScope(_mngr.New<sym::Scope>(nullptr));
    clss->setDependencies(_parameters);

    clss->CanSubtypeClasses::insertParent(clss);

    _created = clss;
}

void ASTDefaultTypeFromKindCreator::visit(TypeConstructorKindSpecifier* tcks) {
    std::vector<TypeExpression*> args(tcks->getArgs().size());
    std::vector<sym::TypeSymbol*> dependencies(tcks->getArgs().size());
    TypeExpression* ret;

    for (size_t i = 0; i < args.size(); ++i) {
        TypeDecl* tdecl = createDefaultTypeFromKind(tcks->getArgs()[i].kindExpr, _name + "Arg" + utils::T_toString(i), _parameters, _ctx);
        args[i] = tdecl->getName();
        dependencies[i] = tdecl->getSymbol();
    }

    pushTypeParameters(dependencies);

    ret = createDefaultTypeFromKind(tcks->getRet(), _name + "Ret", _parameters, _ctx)->getName();

    popTypeParameters(dependencies.size());

    TypeTuple* tt = _mngr.New<TypeTuple>(args);

    TypeConstructorCreation* tcc = _mngr.New<TypeConstructorCreation>(_name, tt, ret);
    tcc->setScope(_mngr.New<sym::Scope>(nullptr));
    tcc->setDependencies(_parameters);

    _created = tcc;
}

TypeExpression* ASTDefaultTypeFromKindCreator::getCreatedType() const {
    return _created;
}

void ASTDefaultTypeFromKindCreator::pushTypeParameters(const std::vector<sym::TypeSymbol*>& typeParams) {
    for (sym::TypeSymbol* typeParam : typeParams) {
        _parameters.push_back(typeParam);
    }
}

void ASTDefaultTypeFromKindCreator::popTypeParameters(size_t pushed) {
    _parameters.resize(_parameters.size() - pushed);
}


TypeDecl* ASTDefaultTypeFromKindCreator::createDefaultTypeFromKind(ASTNode* node, const std::string& name, CompCtx_Ptr& ctx) {
    return createDefaultTypeFromKind(node, name, std::vector<sym::TypeSymbol*>(), ctx);
}

TypeDecl* ASTDefaultTypeFromKindCreator::createDefaultTypeFromKind(ASTNode* node, const std::string& name,
                                               const std::vector<sym::TypeSymbol*>& dependencies, CompCtx_Ptr& ctx) {
    ASTDefaultTypeFromKindCreator creator(ctx, name, dependencies);
    node->onVisit(&creator);
    TypeExpression* expr = creator.getCreatedType();

    type::Type* t = ASTTypeCreator::createType(expr, ctx);

    TypeDecl* tdecl = ctx->memoryManager().New<TypeDecl>(ctx->memoryManager().New<TypeIdentifier>(name), expr);

    sym::TypeSymbol* ts = ctx->memoryManager().New<sym::TypeSymbol>(name, name, tdecl);

    tdecl->setType(t);
    ts->setType(t);

    tdecl->getName()->setSymbol(ts);
    tdecl->setSymbol(ts);

    return tdecl;
}

type::Type* ASTDefaultTypeFromKindCreator::createDefaultTypeFromKind(ASTNode* node, CompCtx_Ptr& ctx) {
    ASTDefaultTypeFromKindCreator creator(ctx, "tmp");
    node->onVisit(&creator);
    TypeExpression* expr = creator.getCreatedType();

    return ASTTypeCreator::createType(expr, ctx);
}

}

}
