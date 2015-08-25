//
//  ASTKindCreator.cpp
//  SFSL
//
//  Created by Romain Beguet on 14.05.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "ASTKindCreator.h"
#include "../Symbols/Scope.h"
#include "ASTTypeCreator.h"

namespace sfsl {

namespace ast {

// AST KIND CREATOR

ASTKindCreator::ASTKindCreator(CompCtx_Ptr& ctx)
    : ASTVisitor(ctx), _created(nullptr) {

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
    std::vector<kind::Kind*> args(tcks->getArgs().size());
    kind::Kind* ret;

    for (size_t i = 0; i < args.size(); ++i) {
        tcks->getArgs()[i]->onVisit(this);
        args[i] = _created;
    }

    tcks->getRet()->onVisit(this);
    ret = _created;

    _created = _mngr.New<kind::TypeConstructorKind>(args, ret);
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
    : ASTVisitor(ctx), _name(name) {

}

ASTDefaultTypeFromKindCreator::~ASTDefaultTypeFromKindCreator() {

}

void ASTDefaultTypeFromKindCreator::visit(ASTNode*) {
    // do not throw an exception
}

void ASTDefaultTypeFromKindCreator::visit(ProperTypeKindSpecifier*) {
    ClassDecl* clss = _mngr.New<ClassDecl>(_name, nullptr, std::vector<TypeSpecifier*>(), std::vector<DefineDecl*>());
    clss->setScope(_mngr.New<sym::Scope>(nullptr));

    clss->CanSubtypeClasses::insertParent(clss);

    _created = clss;
}

void ASTDefaultTypeFromKindCreator::visit(TypeConstructorKindSpecifier* tcks) {
    std::vector<TypeExpression*> args(tcks->getArgs().size());
    TypeExpression* ret;

    for (size_t i = 0; i < args.size(); ++i) {
        tcks->getArgs()[i]->onVisit(this);
        args[i] = _created;
    }

    tcks->getRet()->onVisit(this);
    ret = _created;

    TypeTuple* tt = _mngr.New<TypeTuple>(args);

    TypeConstructorCreation* tcc = _mngr.New<TypeConstructorCreation>(_name, tt, ret);
    tcc->setScope(_mngr.New<sym::Scope>(nullptr));

    _created = tcc;
}

TypeExpression* ASTDefaultTypeFromKindCreator::getCreatedType() const {
    return _created;
}

TypeDecl* ASTDefaultTypeFromKindCreator::createDefaultTypeFromKind(ASTNode* node, const std::string& name, CompCtx_Ptr& ctx) {
    ASTDefaultTypeFromKindCreator creator(ctx, name);
    node->onVisit(&creator);
    TypeExpression* expr = creator.getCreatedType();

    type::Type* t = ASTTypeCreator::createType(expr, ctx);

    TypeDecl* tdecl = ctx->memoryManager().New<TypeDecl>(ctx->memoryManager().New<TypeIdentifier>(name), expr);
    tdecl->setType(t);

    sym::TypeSymbol* ts = ctx->memoryManager().New<sym::TypeSymbol>(name, tdecl);
    ts->setType(t);

    tdecl->setSymbol(ts);

    return tdecl;
}

}

}
